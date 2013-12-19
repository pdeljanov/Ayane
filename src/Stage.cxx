/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "Ayane/Stage.h"
#include "Ayane/Trace.h"

namespace Ayane {
    
    /**
     *  ReconfigureData stores state information used between the
     *  begin/endReconfiguration function pair.
     */
    class StageReconfigurationData {
    public:
        Stage::State mState;
    };
    
    
    class StagePrivate {
    public:
        
        StagePrivate(Stage *q);
        ~StagePrivate();
        
        /** Synchronous processing loop. */
        void syncProcessLoop(Clock*);
        
        /** Begins asynchronous processing. */
        void startAsyncProcess();
        
        /** Stops asynchronous processing. */
        void stopAsyncProcess();
        
        /** Asynchronous processing loop. */
        void asyncProcessLoop();
        
        
        /** Deactivate function without locking. */
        void deactivateNoLock();
        
        /** Stop function without locking. */
        void stopNoLock();
        
        
        /**
         *  Determines if the stage should opeate asynchronously given the
         *  current configuration.
         */
        bool shouldRunAsynchronous() const;
        
        
        void beginReconfiguration(StageReconfigurationData&);
        
        void endReconfiguration(StageReconfigurationData&);
        
        
        Stage *q_ptr;
        AYANE_DECLARE_PUBLIC(Stage);
        
        
        // State tracking.
        std::mutex mStateMutex;
        Stage::State mState;
        
        // Thread for asynchronous processing.
        std::thread mProcessingThread;
        bool mAsynchronousProcessing;
        
        // Clock pointer (If the Stage is running asynchronously, mClock is
        // owned by the Stage and must be freed when stopped. Otherwise,
        // mClock is points to a Clock owned by another Stage and should be
        // reset to null when stopped.
        Clock *mClock;
        uint32_t mBufferQueuesReportedNotFull;
        
        Pipeline *mParentPipeline;
        
        
    };
    
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
    

    
}


using namespace Ayane;


/* StagePrivate */
StagePrivate::StagePrivate(Stage *q) :  q_ptr(q),
                                        mState(Stage::kDeactivated),
                                        mAsynchronousProcessing(false),
                                        mClock(nullptr),
                                        mBufferQueuesReportedNotFull(0)
{
    
}

StagePrivate::~StagePrivate() {
    
    // Transition the state to deactivated.
    {
        std::lock_guard<std::mutex> lock(mStateMutex);
        
        // Stage was playing, transition it to an activated state.
        if( mState == Stage::kPlaying ) {
            
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
            mState = Stage::kActivated;
            
            INFO_THIS("Stage::~Stage") << "Force stopped." << std::endl;
        }
        
        // Deactivate the stage.
        deactivateNoLock();
    }
    
    // Must release the state lock because the sources and sinks are cleared.
    
    A_Q(Stage);
    
    // Clear the sources and sinks before Stage calls the destructors on its
    // member. This allows the sources and sinks to unlink themselves.
    q->mSources.clear();
    q->mSinks.clear();
}

bool StagePrivate::shouldRunAsynchronous() const {
    
    A_Q(const Stage);
    
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
    if( q->sourceCount() == 0 ) {
        return true;
    }
    // If the stage has multiple sources, use async mode. Otherwise, use
    // downstream parameters to determine is async mode is needed.
    else if( q->sourceCount() > 1 ) {
        return true;
    }
    // Only one source, check downstream parameters.
    else
    {
        const Stage::Source *source = q->mSources.begin()->second.get();
        
        if( source->isLinked() ) {
            
            // Check if connected sink is forcing an asynchronous link.
            if(source->mLinkedSink->scheduling() == Stage::Sink::kForceAsynchronous) {
                
                INFO_THIS("Stage::shouldRunAsynchronous") << "Sink: "
                << source->mLinkedSink << " (on Source: " << source
                << ") forcing asynchronous operation." << std::endl;
                
                return true;
            }
            
            // If the downstream Stage contains more than one sink, make the
            // link asynchronous.
            if(source->mLinkedSink->mStage->q_ptr->sinkCount() > 1) {
                return true;
            }
            
        }
    }
    
    return false;
}

void StagePrivate::syncProcessLoop(Clock *clock) {
    
    std::lock_guard<std::mutex> lock(mStateMutex);
    
    if( mState == Stage::kPlaying ) {
        
        // Reset the processing IO flags
        Stage::ProcessIOFlags ioFlags = 0;
        
        // Update the clock pointer so that synchronous process calls can
        // cascade their clocks.
        mClock = clock;
        
        A_Q(Stage);
        
        // Do a process run.
        q->process(&ioFlags);
    }
    else {
        NOTICE_THIS("Stage::syncProcessLoop") << "Attempted to call process() "
        "on a Stage that is not playing." << std::endl;
    }
}

void StagePrivate::asyncProcessLoop() {
    
    A_Q(Stage);

    bool doBufferRun = false;
    Stage::ProcessIOFlags ioFlags = 0;
    uint32_t activeSources = q->mSources.size();
    
    while(doBufferRun || mClock->wait()) {
        
        ioFlags = 0;
        mBufferQueuesReportedNotFull = 0;
        
        {
            // Acquire the state lock, and then do a process run.
            std::lock_guard<std::mutex> lock(mStateMutex);
            
            // Cancellation flag?
            q->process(&ioFlags);
        }
        
        /*
         * Two cases where extra buffering may occur:
         * 1. All sources have reported they can take atleast 1 more buffer.
         * 2. There are no active sources, but the stage is using internal
         *    buffering and it is hinting that it can buffer more.
         */
        doBufferRun = ((mBufferQueuesReportedNotFull > 0) && (mBufferQueuesReportedNotFull == activeSources)) ||
        ((ioFlags & Stage::kProcessMoreHint) && (activeSources == 0));
        
    }
    
    INFO_THIS("Stage::asyncProcessLoop") << "Asynchronous processing thread "
    << std::this_thread::get_id() << " exiting." << std::endl;
}

void StagePrivate::startAsyncProcess(){
    
    if( !mProcessingThread.joinable() ){
        
        // Start the clock.
        mClock->start();
        mProcessingThread = std::thread(&StagePrivate::asyncProcessLoop, this);
        
        INFO_THIS("Stage::startAsyncProcess") << "Started asynchronous "
        "processing thread " << mProcessingThread.get_id() << "." << std::endl;
    }
    else
    {
        NOTICE_THIS("Stage::startAsyncProcess") << "Asynchronous processing "
        "thread already started." << std::endl;
    }
    
}

void StagePrivate::stopAsyncProcess() {
    
    if( mProcessingThread.joinable() ){
        
        TRACE_THIS("Stage::stopAsyncProcess") << "Waiting for asynchronous "
        "processing thread to stop." << std::endl;
        
        // Stop the clock. Processing thread will exit.
        mClock->stop();
        mProcessingThread.join();
    }
}

void StagePrivate::deactivateNoLock() {
    
    A_Q(Stage);
    
    // If the stage is in the playing state, stop playback.
    if( mState == Stage::kPlaying ) {
        // Stop playback. Use no-lock variant since we have the state lock.
        stopNoLock();
    }
    
    // If the stage is activated, proceed to deactivate it.
    if( mState == Stage::kActivated ) {
        
        // Reset sources (clears synchronicity and empties buffers).
        for(Stage::SourceIterator iter = q->mSources.begin(), end = q->mSources.end();
            iter != end; ++iter)
        {
            q->resetPort(iter->second.get());
        }
        
        // Remove pipeline.
        mParentPipeline = nullptr;
        
        // Record state.
        mState = Stage::kDeactivated;
        
        INFO_THIS("Stage::deactivate") << "Deactivated." << std::endl;
    }
}

void StagePrivate::stopNoLock() {
    
    A_Q(Stage);
    
    if( mState == Stage::kPlaying ) {
        
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
        q->stoppedPlayback();
        
        // Record the state.
        mState = Stage::kActivated;
    }
}

void StagePrivate::beginReconfiguration(StageReconfigurationData &data) {
    
    // Lock the state mutex. This will prevent any process() runs.
    mStateMutex.lock();
    
    // Store previous state.
    data.mState = mState;
}

void StagePrivate::endReconfiguration(StageReconfigurationData& data) {
    
    // If the previou state was playing, call the online-reconfiguration
    // handler.
    if( data.mState == Stage::kPlaying ) {
        A_Q(Stage);
        q->reconfigureIO();
    }
    
    // Unlock the state mutex. Process() runs can occur after this.
    mStateMutex.unlock();
}

/* Stage */

Stage::Stage() : d_ptr(new StagePrivate(this)){
    
}

Stage::~Stage() {
    delete d_ptr;
}

const Clock* Stage::clock() const {
    A_D(const Stage);
    return d->mClock;
}

Pipeline* Stage::pipeline() const {
    A_D(const Stage);
    return d->mParentPipeline;
}

Stage::State Stage::state() const {
    A_D(const Stage);
    return d->mState;
}

bool Stage::activate(Pipeline *pipeline) {
    
    A_D(Stage);
    
    std::lock_guard<std::mutex> lock(d->mStateMutex);
    
    // Deactivated -> Activated.
    if( d->mState == kDeactivated ) {
        
        // Set the pipeline.
        d->mParentPipeline = pipeline;
        
        // Switch state to activated.
        d->mState = kActivated;
        
        INFO_THIS("Stage::activate") << "Activated." << std::endl;
        
        return true;
    }
    
    return false;
}

void Stage::deactivate() {
    
    A_D(Stage);
    
    std::lock_guard<std::mutex> lock(d->mStateMutex);
    d->deactivateNoLock();
}

void Stage::play(ClockProvider &clockProvider) {
    
    A_D(Stage);
    
    std::lock_guard<std::mutex> lock(d->mStateMutex);

    // Activated (Stopped) -> Playing
    if( d->mState == kActivated ) {
        
        // Determine synchronicity.
        d->mAsynchronousProcessing = d->shouldRunAsynchronous();
        
        TRACE_THIS("Stage::play") << "Stage will run "
        << (d->mAsynchronousProcessing ? "asynchronously." : "synchronously.")
        << std::endl;
        
        // Assign synchronicity mode to the sources.
        SynchronicityMode mode = (d->mAsynchronousProcessing) ? kAsynchronous : kSynchronous;
        
        for(SourceIterator iter = mSources.begin(), end = mSources.end();
            iter != end; ++iter)
        {
            iter->second->mShared->mLinkSynchronicity = mode;
        }
        
        // Start the clock
        // NOTE: Clock must be started before beginPlayback().
        if( d->mAsynchronousProcessing ){
            d->mClock = new Clock;
            clockProvider.registerClock(static_cast<Clock*>(d->mClock));
        }

        // Begin playback callback.
        // NOTE: Must occur before buffers are processing.
        beginPlayback();
 
        // If asynchronous, start the processing thread.
        if( d->mAsynchronousProcessing ) {
            d->startAsyncProcess();
        }
        
        // Record the state.
        d->mState = kPlaying;
        
        INFO_THIS("Stage::play") << "Playing." << std::endl;
    }
    
}

void Stage::stop() {
    
    A_D(Stage);
    
    std::lock_guard<std::mutex> lock(d->mStateMutex);
    d->stopNoLock();
}


void Stage::push(Source *source, ManagedBuffer &buffer)
{
    SourceSinkPrivate *shared = source->mShared.get();
    
    if( !shared->mBufferQueue.push(buffer) ) {
        // Buffer couldn't be inserted due to the queue being full. This should
        // not happen unless the downstream sink isn't working properly.
        WARNING_THIS("Stage::Source::push") << "Failed to push buffer." << std::endl;
        return;
    }
    
    if( shared->mLinkSynchronicity == kAsynchronous ) {
        
        // Report if the buffer queue is not full.
        if( !shared->mBufferQueue.full() ) {
            A_D(Stage);
            d->mBufferQueuesReportedNotFull++;
        }
        
        shared->mPushNotification.notify_one();
    }
}

Stage::PullResult Stage::pull(Sink *sink, ManagedBuffer *outBuffer )
{
    SourceSinkPrivate *shared = sink->mShared;
    
    switch(shared->mLinkSynchronicity) {
        case kAsynchronous: {
            
            std::unique_lock<std::mutex> lock(shared->mPushMutex);
            
            // Wait for a buffer to be pushed into the queue.
            while(shared->mBufferQueue.empty()) {
                shared->mPushNotification.wait(lock);
                
                if( sink->mPullCancelled ) {
                    sink->mPullCancelled = false;
                    return kCancelled;
                }
            }
        }
        case kSynchronous: {
            
            A_D(Stage);

            sink->mLinkedSource->mStage->syncProcessLoop(d->mClock);
        }
    }

    if( !shared->mBufferQueue.pop(outBuffer) ) {
        return kBufferQueueEmpty;
    }
    
    // Check if the buffer's format matches the sink's format.
    if( (*outBuffer)->format() != sink->mBufferFormat ) {
        if( !reconfigureInputFormat(*sink, (*outBuffer)->format()) ){
            return kUnsupportedFormat;
        }
        
        // Format accepted.
        sink->mBufferFormat = (*outBuffer)->format();
    }
    
    return kSuccess;
}

Stage::PullResult Stage::tryPull(Sink *sink, ManagedBuffer *outBuffer) {
    
    SourceSinkPrivate *shared = sink->mShared;
    
    if( shared->mLinkSynchronicity == kAsynchronous ) {
        if( !shared->mBufferQueue.pop(outBuffer) ) {
            return kBufferQueueEmpty;
        }
    }
    else {
        // tryPull makes no sense on synchronous sources because we can't
        // control the types of pulls performed upstream.
        return kNotAsynchronous;
    }
    
    // Check if the buffer's format matches the sink's format.
    if( (*outBuffer)->format() != sink->mBufferFormat ) {
        if( !reconfigureInputFormat(*sink, (*outBuffer)->format()) ){
            return kUnsupportedFormat;
        }
        
        // Format accepted.
        sink->mBufferFormat = (*outBuffer)->format();
    }
    
    return kSuccess;
}

void Stage::cancelPull(Sink *sink)
{
    SourceSinkPrivate *shared = sink->mShared;
    
    // No-op in synchronous mode.
    if( shared->mLinkSynchronicity == kAsynchronous ) {
        
        std::lock_guard<std::mutex> lock(shared->mPushMutex);
        
        // Set cancellation flag.
        sink->mPullCancelled = true;
        
        // Notify any waiting pulls that it can cancel its wait.
        shared->mPushNotification.notify_one();
    }
}

void Stage::resetPort(Source *source) {
    // Clear the queue.
    source->mShared->mBufferQueue.clear();
}

void Stage::resetPort(Sink *sink) {
    sink->mBufferFormat = BufferFormat();
}



void Stage::addSource(const std::string &name) {
    
    A_D(Stage);
    
    if( d->mState == kDeactivated ){
        mSources.insert(std::make_pair(name, std::unique_ptr<Source>(new Source(d))));
    }
    else {
        NOTICE_THIS("Stage::addSource") << "Can't add source unless stage is "
        "deactivated." << std::endl;
    }
}

void Stage::addSink(const std::string &name) {

    A_D(Stage);

    if( d->mState == kDeactivated ) {
        mSinks.insert(std::make_pair(name, std::unique_ptr<Sink>(new Sink(d))));
    }
    else {
        NOTICE_THIS("Stage::addSink") << "Can't add sink unless stage is "
        "deactivated." << std::endl;
    }
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

        StageReconfigurationData sinkData, currentSourceData, nextSourceData;
        
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
        
        StageReconfigurationData sinkData, sourceData;
        
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

        StageReconfigurationData sinkData, sourceData;
        
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

Stage::Source::Source(StagePrivate *stage) :
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



/* Stage::Sink */

Stage::Sink::Sink(StagePrivate *stage) :
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


