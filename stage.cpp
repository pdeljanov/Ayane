/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "stage.h"
#include "trace.h"

namespace Stargazer {
    namespace Audio  {
        
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
}


using namespace Stargazer::Audio;

/* Stage */

Stage::Stage() :
    mState(kDeactivated),
    mAsynchronousProcessing(false),
    mClock(nullptr),
    mBufferQueuesReportedNotFull(0)
{
    
}

Stage::~Stage()
{
    // Stage was "killed"
    deactivate();
    
    // Clear the sources and sinks before Stage calls the destructors on its
    // member. This allows the sources and sinks to unlink themselves.
    mSources.clear();
    mSinks.clear();
}

void Stage::syncProcessLoop() {
    /*
     *  Unlike the asynchronous processing loop which is directly
     *  controlled by this Stage, a synchronous process loop is 
     *  controlled by the upstream sink.  Therefore, before any state
     *  changes can be made to Stage, the synchronous process loop
     *  must finish its run. To accomplish this, obtain the state mutex
     *  lock.
     */
    std::lock_guard<std::mutex> lock(mStateMutex);
    
    if( mState == kPlaying ) {
        ProcessIOFlags ioFlags = 0;
        process(&ioFlags);
    }
    else {
        NOTICE_THIS("Stage::syncProcessLoop") << "Attempted to call process() "
        "on a Stage that is not playing." << std::endl;
    }
}

void Stage::asyncProcessLoop()
{
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

void Stage::startAsyncProcess()
{
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

void Stage::stopAsyncProcess()
{
    if( mProcessingThread.joinable() ){
        
        TRACE_THIS("Stage::stopAsyncProcess") << "Waiting for asynchronous "
        "processing thread to stop." << std::endl;
                
        // Stop the clock. Processing thread will exit
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
            if( source->mLinkedSink->mStage->sinkCount() > 1 ) {
                return true;
            }
            
        }
    }
    
    return false;
}


bool Stage::activate()
{
    std::lock_guard<std::mutex> lock(mStateMutex);
    
    // Deactivated -> Activated.
    if( mState == kDeactivated ) {
        
        // Switch state to activated.
        mState = kActivated;
        
        INFO_THIS("Stage::activate") << "Activated." << std::endl;
        
        return true;
    }
    
    return false;
}

void Stage::deactivate()
{
    std::lock_guard<std::mutex> lock(mStateMutex);

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
        
        // Record state.
        mState = kDeactivated;
        
        INFO_THIS("Stage::deactivate") << "Deactivated." << std::endl;
    }
}

void Stage::play( AbstractClock *clock )
{
    std::lock_guard<std::mutex> lock(mStateMutex);

    // Activated (Stopped) -> Playing
    if( mState == kActivated ) {
        
        // Set the clock.
        mClock.reset(clock);
        
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

        // Begin playback callback. Must occur before buffers are processed!
        beginPlayback();
 
        if( mAsynchronousProcessing ) {
            // If asynchronous, start the processing thread.
            startAsyncProcess();
        }
        else {
            // Start the clock manually in synchronous mode.
            mClock->start();
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

void Stage::stopNoLock()
{
    if( mState == kPlaying ) {
        
        if( mAsynchronousProcessing ) {
            // If asynchronous, stop the processing thread.
            // Wait till processing stops.
            stopAsyncProcess();
        }
        else {
            // If synchronous, stop the clock manually.
            mClock->stop();
        }
        
        // Playback stopped callback. Must occur after all buffers are
        // processed.
        stoppedPlayback();

        // Record the state.
        mState = kActivated;
    }
}



void Stage::addSource(const std::string &name)
{
    if( mState == kDeactivated ){
        mSources.insert( std::make_pair(name, std::unique_ptr<Source>(new Source(this))) );
    }
    else {
        NOTICE_THIS("Stage::addSource") << "Can't add source unless stage is "
        "deactivated." << std::endl;
    }
}

void Stage::addSink(const std::string &name)
{
    if( mState == kDeactivated ) {
        mSinks.insert( std::make_pair(name, std::unique_ptr<Sink>(new Sink(this))) );
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
        << " <-----> " << sink->mStage    <<  ":" << sink
        << " <-/ /-> " << current->mStage <<  ":" << current
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
        << source << " <-----> " << sink->mStage <<  ":" << sink << std::endl;
        
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
        << source << " <-/ /-> " << sink->mStage <<  ":" << sink << std::endl;
        
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
            while( mShared->mBufferQueue.empty() ) {
                mShared->mPushNotification.wait(lock);

                if( mPullCancelled ) {
                    mPullCancelled = false;
                    return kCancelled;
                }
            }
        }
        case kSynchronous: {
            // Generate a buffer only if one isn't already pending.
            if( mShared->mBufferQueue.empty() ) {
                mLinkedSource->mStage->syncProcessLoop();
            }
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
