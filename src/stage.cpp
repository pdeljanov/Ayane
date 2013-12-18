/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "stage.h"
#include "trace.h"

namespace Ayane {
        
    /**
     *  SourceSinkPrivate stores the shared state between a linked Source 
     *  and Sink.
     */
    class Stage::SourceSinkPrivate {
        
    public:
        
        SourceSinkPrivate();
        ~SourceSinkPrivate();
        
        SynchronicityMode mLinkSynchronicity;
        
        BufferQueue mBufferQueue;
        
        std::mutex mPushMutex;
        std::condition_variable mPushNotification;
        
    };
    
    /**
     *  ReconfigureData stores state information used between the
     *  begin/endReconfiguration function pair.
     */
    class Stage::ReconfigureData {
    public:
        State mState;
    };
    
}


using namespace Ayane;

/* Stage */

Stage::Stage() :
    mState(kDeactivated),
    mAsynchronousProcessing(false),
    mClock(nullptr),
    mBufferQueuesReportedNotFull(0)
{
    
}

Stage::~Stage() {
    
    // Transition the state to deactivated.
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        
        // Stage was playing, transition it to an activated state.
        if( mState == kPlaying ) {
            
            WARNING_THIS("Stage::~Stage") << "It is **highly unrecommended** to"
            " call the destructor of a playing stage. Call stop() first."
            << std::endl;
            
            if( mAsynchronousProcessing ) {
                stopAsyncProcess();
                
                if(mClock){
                    delete mClock;
                }
            }
            
            // Reset clock pointer.
            mClock = nullptr;
            
            // Record the state.
            mState = kActivated;
            
            INFO_THIS("Stage::~Stage") << "Force stopped." << std::endl;
        }
        
        // Deactivate the stage.
        deactivateNoLock();
    }
    
    // Must release the state lock because the sources and sinks are cleared.
    
    // Clear the sources and sinks before Stage calls the destructors on its
    // member. This allows the sources and sinks to unlink themselves.
    mSources.clear();
    mSinks.clear();
}

void Stage::syncProcessLoop(Clock *clock) {
    
    std::lock_guard<std::mutex> lock(mStateMutex);
    
    if( mState == kPlaying ) {
        
        // Reset the processing IO flags
        ProcessIOFlags ioFlags = 0;
        
        // Update the clock pointer so that synchronous process calls can
        // cascade their clocks.
        mClock = clock;
        
        // Do a process run.
        process(&ioFlags);
    }
    else {
        NOTICE_THIS("Stage::syncProcessLoop") << "Attempted to call process() "
        "on a Stage that is not playing." << std::endl;
    }
}

void Stage::asyncProcessLoop() {
    
    bool doBufferRun = false;
    ProcessIOFlags ioFlags = 0;
    uint32_t activeSources = mSources.size();

    
    while(doBufferRun || mClock->wait()) {
        
        ioFlags = 0;
        mBufferQueuesReportedNotFull = 0;
        
        {
            // Acquire the state lock, and then do a process run.
            std::lock_guard<std::mutex> lock(mStateMutex);
            
            // Cancellation flag?
            process(&ioFlags);
        }
        
        /*
         * Two cases where extra buffering may occur:
         * 1. All sources have reported they can take atleast 1 more buffer.
         * 2. There are no active sources, but the stage is using internal
         *    buffering and it is hinting that it can buffer more.
         */
        doBufferRun = ((mBufferQueuesReportedNotFull > 0) && (mBufferQueuesReportedNotFull == activeSources)) ||
                      ((ioFlags & kProcessMoreHint) && (activeSources == 0));

    }

    INFO_THIS("Stage::asyncProcessLoop") << "Asynchronous processing thread "
    << std::this_thread::get_id() << " exiting." << std::endl;
}

void Stage::startAsyncProcess(){
    
    if( !mProcessingThread.joinable() ){

        // Start the clock.
        mClock->start();
        mProcessingThread = std::thread( &Stage::asyncProcessLoop, this );
        
        INFO_THIS("Stage::startAsyncProcess") << "Started asynchronous "
        "processing thread " << mProcessingThread.get_id() << "." << std::endl;
    }
    else
    {
        NOTICE_THIS("Stage::startAsyncProcess") << "Asynchronous processing "
        "thread already started." << std::endl;
    }

}

void Stage::stopAsyncProcess() {
    
    if( mProcessingThread.joinable() ){
        
        TRACE_THIS("Stage::stopAsyncProcess") << "Waiting for asynchronous "
        "processing thread to stop." << std::endl;

        // Stop the clock. Processing thread will exit.
        mClock->stop();
        mProcessingThread.join();
    }
}

bool Stage::shouldRunAsynchronous() const {
    
    /* Synchonicity mode
     *
     * The synchonicity mode of the stage is determined by the scheduling
     * mode of downstream sink ports, and the number of source ports.
     *
     *                       |  # of Sinks (receiver)
     *  ---------------------+-----------+------------
     *                       |    One    |   Many
     *  ----------------------------------------------
     *  # of Sources | One   |   Sync    |   Async
     *    (sender)   | Many  |   Async   |   Async
     *
     * However, if a source's sink is in ForceAsynchonous mode, the stage
     * will always operate asynchonously.
     *
     */
    
    // Pure sink node.
    if( sourceCount() == 0 ) {
        return true;
    }
    // If the stage has multiple sources, use async mode. Otherwise, use
    // downstream parameters to determine is async mode is needed.
    else if( sourceCount() > 1 ) {
        return true;
    }
    // Only one source, check downstream parameters.
    else
    {
        const Source *source = mSources.begin()->second.get();
        
        if( source->isLinked() ) {

            // Check if connected sink is forcing an asynchronous link.
            if(source->mLinkedSink->scheduling() == Sink::kForceAsynchronous) {
                
                INFO_THIS("Stage::shouldRunAsynchronous") << "Sink: "
                << source->mLinkedSink << " (on Source: " << source
                << ") forcing asynchronous operation." << std::endl;
                
                return true;
            }
            
            // If the downstream Stage contains more than one sink, make the
            // link asynchronous.
            if(source->mLinkedSink->mStage->sinkCount() > 1) {
                return true;
            }
            
        }
    }
    
    return false;
}


bool Stage::activate(Pipeline *pipeline) {
    
    std::lock_guard<std::mutex> lock(mStateMutex);
    
    // Deactivated -> Activated.
    if( mState == kDeactivated ) {
        
        // Set the pipeline.
        mParentPipeline = pipeline;
        
        // Switch state to activated.
        mState = kActivated;
        
        INFO_THIS("Stage::activate") << "Activated." << std::endl;
        
        return true;
    }
    
    return false;
}

void Stage::deactivateNoLock() {
    
    // If the stage is in the playing state, stop playback.
    if( mState == kPlaying ) {
        // Stop playback. Use no-lock variant since we have the state lock.
        stopNoLock();
    }
    
    // If the stage is activated, proceed to deactivate it.
    if( mState == kActivated ) {
        
        // Reset sources (clears synchronicity and empties buffers).
        for(SourceIterator iter = mSources.begin(), end = mSources.end();
            iter != end; ++iter)
        {
            iter->second->reset();
        }
        
        // Remove pipeline.
        mParentPipeline = nullptr;
        
        // Record state.
        mState = kDeactivated;
        
        INFO_THIS("Stage::deactivate") << "Deactivated." << std::endl;
    }
}

void Stage::deactivate() {
    std::lock_guard<std::mutex> lock(mStateMutex);
    deactivateNoLock();
}

void Stage::play(ClockProvider &clockProvider) {
    
    std::lock_guard<std::mutex> lock(mStateMutex);

    // Activated (Stopped) -> Playing
    if( mState == kActivated ) {
        
        // Determine synchronicity.
        mAsynchronousProcessing = shouldRunAsynchronous();
        
        TRACE_THIS("Stage::play") << "Stage will run "
        << (mAsynchronousProcessing ? "asynchronously." : "synchronously.")
        << std::endl;
        
        // Assign synchronicity mode to the sources.
        SynchronicityMode mode = (mAsynchronousProcessing) ? kAsynchronous : kSynchronous;
        
        for(SourceIterator iter = mSources.begin(), end = mSources.end();
            iter != end; ++iter)
        {
            iter->second->mShared->mLinkSynchronicity = mode;
        }
        
        // Start the clock
        // NOTE: Clock must be started before beginPlayback().
        if( mAsynchronousProcessing ){
            mClock = new Clock;
            clockProvider.registerClock(static_cast<Clock*>(mClock));
        }

        // Begin playback callback.
        // NOTE: Must occur before buffers are processing.
        beginPlayback();
 
        // If asynchronous, start the processing thread.
        if( mAsynchronousProcessing ) {
            startAsyncProcess();
        }
        
        // Record the state.
        mState = kPlaying;
        
        INFO_THIS("Stage::play") << "Playing." << std::endl;
    }
    
}

void Stage::stop() {
    std::lock_guard<std::mutex> lock(mStateMutex);
    stopNoLock();
}

void Stage::stopNoLock() {
    
    if( mState == kPlaying ) {
        
        if( mAsynchronousProcessing ) {
            // If asynchronous, stop the processing thread.
            // Wait till processing stops.
            stopAsyncProcess();
            
            // We own the clock, so delete it.
            if (mClock) {
                delete mClock;
            }
        }
        
        // Set the clock pointer to null.
        // NOTE: Even if running synchronously, the clock pointer needs to be
        // reset to null so that the Stage won't free the un-owned clock.
        mClock = nullptr;
        
        // Playback stopped callback. Must occur after all buffers are
        // processed.
        stoppedPlayback();

        // Record the state.
        mState = kActivated;
    }
}



void Stage::addSource(const std::string &name) {
    
    if( mState == kDeactivated ){
        mSources.insert(std::make_pair(name,
                                       std::unique_ptr<Source>(new Source(this))));
    }
    else {
        NOTICE_THIS("Stage::addSource") << "Can't add source unless stage is "
        "deactivated." << std::endl;
    }
}

void Stage::addSink(const std::string &name) {
    if( mState == kDeactivated ) {
        mSinks.insert(std::make_pair(name,
                                     std::unique_ptr<Sink>(new Sink(this))));
    }
    else {
        NOTICE_THIS("Stage::addSink") << "Can't add sink unless stage is "
        "deactivated." << std::endl;
    }
}




void Stage::beginReconfiguration(ReconfigureData &data) {

    // Lock the state mutex. This will prevent any process() runs.
    mStateMutex.lock();

    // Store previous state.
    data.mState = mState;
}

void Stage::endReconfiguration(ReconfigureData& data) {
    
    // If the previou state was playing, call the online-reconfiguration
    // handler.
    if( data.mState == kPlaying ) {
        reconfigureIO();
    }
    
    // Unlock the state mutex. Process() runs can occur after this.
    mStateMutex.unlock();
}



bool Stage::replace(Source *current, Source *next, Sink *sink) {
    
    // Null pointer check.
    if((current == nullptr) || (next == nullptr) || (sink == nullptr)) {
        ERROR("Stage::replace") << "Attempting to relink null pointer."
        << std::endl;
        return false;
    }
    
    // Check if trying to replace the current source with itself.
    if( current == next ) {
        
        NOTICE("Stage::replace") << "Trying to replace source " << current
        << " with itself on " << sink->mStage << ":" << sink << "."
        << std::endl;
        
        return true;
    }
    
    // Only replace if the ports are linked to each other.
    if( (current->mLinkedSink == sink) && (sink->mLinkedSource == current) ) {

        ReconfigureData sinkData, currentSourceData, nextSourceData;
        
        sink->mStage->beginReconfiguration(sinkData);
        current->mStage->beginReconfiguration(currentSourceData);
        next->mStage->beginReconfiguration(nextSourceData);

        // Unlink from current source
        current->mLinkedSink = nullptr;

        // Link to new source
        sink->mShared = next->mShared.get();
        sink->mLinkedSource = next;
        next->mLinkedSink = sink;
        
        next->mStage->endReconfiguration(nextSourceData);
        sink->mStage->endReconfiguration(sinkData);
        current->mStage->endReconfiguration(currentSourceData);

        INFO("Stage::replace") << "Relinked: " << next->mStage << ":" << next
        << " +-----> " << sink->mStage    <<  ":" << sink
        << " <-/ /-+ " << current->mStage <<  ":" << current
        << std::endl;
        
        return true;
    }
    else
    {
        return false;
    }
}


bool Stage::link( Source *source, Sink *sink )
{
    // Null pointer check.
    if( (source == nullptr) || (sink == nullptr) ) {
        ERROR("Stage::link") << "Attempting to link null pointer." << std::endl;
        return false;
    }
    
    if( (source->mLinkedSink == nullptr) && (sink->mLinkedSource == nullptr) ) {
        
        ReconfigureData sinkData, sourceData;
        
        sink->mStage->beginReconfiguration(sinkData);
        source->mStage->beginReconfiguration(sourceData);
        
        // Perform link.
        source->mLinkedSink = sink;
        sink->mLinkedSource = source;
        sink->mShared = source->mShared.get();
        
        source->mStage->endReconfiguration(sourceData);
        sink->mStage->endReconfiguration(sinkData);
        
        INFO("Stage::link") << "Linked: " << source->mStage << ":"
        << source << " +-----> " << sink->mStage <<  ":" << sink << std::endl;
        
        return true;
    }
    else {
        NOTICE("Stage::link") << "Source or sink already linked." << std::endl;
        return false;
    }
}

void Stage::unlink( Source *source, Sink *sink )
{
    // Null pointer check.
    if( (source == nullptr) || (sink == nullptr) ) {
        ERROR("Stage::unlink") << "Attempting to unlink null pointer."
        << std::endl;
        return;
    }
    
    // Only unlink if the ports are linked to each other.
    if( (source->mLinkedSink == sink) && (sink->mLinkedSource == source) ) {

        ReconfigureData sinkData, sourceData;
        
        sink->mStage->beginReconfiguration(sinkData);
        source->mStage->beginReconfiguration(sourceData);
        
        // Unlink
        sink->mShared = nullptr;
        source->mLinkedSink = nullptr;
        sink->mLinkedSource = nullptr;
        
        source->mStage->endReconfiguration(sourceData);
        sink->mStage->endReconfiguration(sinkData);
        
        INFO("Stage::unlink") << "Unlinked: " << source->mStage << ":"
        << source << " +-/ /-> " << sink->mStage <<  ":" << sink << std::endl;
        
    }
    else {
        NOTICE("Stage::unlink") << "Source: " << source << " not linked to "
        "sink: " << sink << std::endl;
    }
}


/* Stage::SourceSinkPrivate */

Stage::SourceSinkPrivate::SourceSinkPrivate() :
    mLinkSynchronicity(kSynchronous),
    mBufferQueue(2)
{
    
}

Stage::SourceSinkPrivate::~SourceSinkPrivate() {
    
}


/* Stage::Source */

Stage::Source::Source( Stage *stage ) :
    mStage(stage),
    mLinkedSink(nullptr),
    mShared(new Stage::SourceSinkPrivate)
{
    
}

Stage::Source::~Source() {
    
    // Unlink if necessary.
    if( isLinked() ){
        Stage::unlink(this, mLinkedSink);
    }

}

bool Stage::Source::isLinked() const {
    return (mLinkedSink != nullptr);
}

Stage::SynchronicityMode Stage::Source::linkSynchronicity() const {
    return mShared->mLinkSynchronicity;
}

bool Stage::Source::checkFormatSupport(const BufferFormat &format) const
{
    if( !mLinkedSink->checkFormatSupport(format) ) {
        return true;
    }
    
    return false;
}

void Stage::Source::push(ManagedBuffer &buffer)
{
    if( !mShared->mBufferQueue.push(buffer) ) {
        // Buffer couldn't be inserted due to the queue being full. This should
        // not happen unless the downstream sink isn't working properly.
        WARNING_THIS("Stage::Source::push") << "Failed to push buffer." << std::endl;
        return;
    }
    
    if( mShared->mLinkSynchronicity == kAsynchronous ) {
        
        // Report if the buffer queue is not full.
        if( !mShared->mBufferQueue.full() ) {
            mStage->reportBufferQueueIsNotFull();
        }
        
        mShared->mPushNotification.notify_one();
    }
    

}


void Stage::Source::reset() {
    // Clear the queue.
    mShared->mBufferQueue.clear();
}



/* Stage::Sink */

Stage::Sink::Sink( Stage *stage ) :
    mStage(stage),
    mLinkedSource(nullptr),
    mShared(nullptr),
    mBufferFormat(),
    mPullCancelled(false)
{
    
}

Stage::Sink::~Sink() {
    
    // Unlink if necessary.
    if( isLinked() ){
        Stage::unlink(mLinkedSource, this);
    }
    
}

void Stage::Sink::reset() {
    mBufferFormat = BufferFormat();
}

bool Stage::Sink::isLinked() const {
    return (mLinkedSource != nullptr);
}

Stage::SynchronicityMode Stage::Sink::linkSynchronicity() const {
    return mShared->mLinkSynchronicity;
}

bool Stage::Sink::checkFormatSupport( const BufferFormat &format ) const
{
#pragma unused(format)
    // TODO: Implement range based format checking.
    return true;
}

Stage::Sink::PullResult Stage::Sink::pull( ManagedBuffer *outBuffer )
{
    switch(mShared->mLinkSynchronicity) {
        case kAsynchronous: {
            
            std::unique_lock<std::mutex> lock(mShared->mPushMutex);
            
            // Wait for a buffer to be pushed into the queue.
            while(mShared->mBufferQueue.empty()) {
                mShared->mPushNotification.wait(lock);

                if( mPullCancelled ) {
                    mPullCancelled = false;
                    return kCancelled;
                }
            }
        }
        case kSynchronous: {
            // TODO: Should there be a check if the buffer queue is empty?
            mLinkedSource->mStage->syncProcessLoop(mStage->mClock);
        }
    }
    
    if( !mShared->mBufferQueue.pop(outBuffer) ) {
        return kBufferQueueEmpty;
    }
    
    // Check if the buffer's format matches the sink's format.
    if( (*outBuffer)->format() != mBufferFormat ) {
        if( !mStage->reconfigureInputFormat(*this, (*outBuffer)->format()) ){
            return kUnsupportedFormat;
        }
        
        // Format accepted.
        mBufferFormat = (*outBuffer)->format();
    }
    
    return kSuccess;
}

Stage::Sink::PullResult Stage::Sink::tryPull(ManagedBuffer *outBuffer) {
    
    if( mShared->mLinkSynchronicity == kAsynchronous ) {
        if( !mShared->mBufferQueue.pop(outBuffer) ) {
            return kBufferQueueEmpty;
        }
    }
    else {
        // tryPull makes no sense on synchronous sources because we can't
        // control the types of pulls performed upstream.
        return kNotAsynchronous;
    }
    
    // Check if the buffer's format matches the sink's format.
    if( (*outBuffer)->format() != mBufferFormat ) {
        if( !mStage->reconfigureInputFormat(*this, (*outBuffer)->format()) ){
            return kUnsupportedFormat;
        }
        
        // Format accepted.
        mBufferFormat = (*outBuffer)->format();
    }
    
    return kSuccess;
}

void Stage::Sink::cancelPull()
{
    // No-op in synchronous mode.
    if( mShared->mLinkSynchronicity == kAsynchronous ) {
        
        std::lock_guard<std::mutex> lock(mShared->mPushMutex);
        
        // Set cancellation flag.
        mPullCancelled = true;
        
        // Notify any waiting pulls that it can cancel its wait.
        mShared->mPushNotification.notify_one();
    }
}
