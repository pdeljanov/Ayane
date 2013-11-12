#include "stage.h"

#include <iostream>


namespace Stargazer {
    namespace Audio  {
        
        
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
}

void Stage::asyncProcessLoop()
{
    bool doBufferRun = false;
    ProcessIOFlags ioFlags = 0;
    uint32_t activeSources = mSources.size();

    
    while(doBufferRun || mClock->wait()) {

        /*
        std::cout << "Stage::asyncProcessLoop: "
        << (doBufferRun ? "Buffered" : "Clocked") << " run."
        << std::endl;
        */
        
        ioFlags = 0;
        mBufferQueuesReportedNotFull = 0;
        
        process(&ioFlags);

        /*
         * Two cases where extra buffering may occur:
         * 1. All sources have reported they can take atleast 1 more buffer.
         * 2. There are no active sources, but the stage is using internal
         *    buffering and it is hinting that it can buffer more.
         */
        doBufferRun = ((mBufferQueuesReportedNotFull > 0) && (mBufferQueuesReportedNotFull == activeSources)) ||
                      ((ioFlags & kProcessMoreHint) && (activeSources == 0));

    }

    std::cout << "Stage::asyncProcessLoop: Processing thread with ID: "
    << mProcessingThread.get_id() << " done." << std::endl;
}

void Stage::startAsyncProcess()
{
    if( !mProcessingThread.joinable() ){

        // Start the clock.
        mClock->start();
        mProcessingThread = std::thread( &Stage::asyncProcessLoop, this );
        
        std::cout << "Stage::startAsyncProcess: Started thread with ID: "
        << mProcessingThread.get_id() << "." << std::endl;
    }
    else
    {
        std::cout << "Stage::startAsyncProcess: Thread already started with ID: "
        << mProcessingThread.get_id() << "." << std::endl;
    }

}

void Stage::stopAsyncProcess()
{
    if( mProcessingThread.joinable() ){
        
        std::cout << "Stage::stopAsyncProcess: Waiting for processing thread to"
        " stop." << std::endl;
        
        // Stop the clock. Processing thread will exit
        mClock->stop();
        mProcessingThread.join();
        
        std::cout << "Stage::stopAsyncProcess: Processing thread stopped."
        << std::endl;
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
    else
    {
        for(ConstSourceIterator iter = mSources.begin(), end = mSources.end();
            iter != end; ++iter)
        {
            const Sink *sink = iter->second->mLinkedSink;
            
            // Ignore if source is unlinked.
            if( sink == nullptr ) {
                continue;
            }
            
            // If a downstream sink forces async operation, use async mode.
            if(sink->scheduling() == Sink::kForceAsynchronous) {
                
                std::cout << "Stage::shouldRunAsynchronous: Sink: " << sink
                << " (on Source: " << iter->first
                << ") forcing asynchronous operation." << std::endl;
                
                return true;
            }
            
            // If a downstream sink has multiple sinks, use async mode.
            if( sink->mStage->sinkCount() > 1 ) {
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
        
        std::cout << "Stage::activate: Stage (" << this << ") activated."
        << std::endl;
        
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
        
        std::cout << "Stage::deactivate: Stage (" << this << ") deactivated."
        << std::endl;
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
        
        std::cout << "Stage::play: Stage (" << this << ") will run "
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
        
        std::cout << "Stage::play: Stage (" << this << ") playing."
        << std::endl;
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
        mSources.insert( std::make_pair(name, new Source(this)) );
    }
    else {
        std::cout << "Stage::addSource: Can't add source unless stage is "
        "deactivated." << std::endl;
    }
}

void Stage::addSink(const std::string &name)
{
    if( mState == kDeactivated ) {
        mSinks.insert( std::make_pair(name, new Sink(this)) );
    }
    else {
        std::cout << "Stage::addSink: Can't add sink unless stage is "
        "deactivated." << std::endl;
    }
}


bool Stage::link( Source *source, Sink *sink )
{
    if( (source->mLinkedSink == nullptr) && (sink->mLinkedSource == nullptr) ) {
        
        source->mLinkedSink = sink;
        sink->mLinkedSource = source;

        // Link the shared state.
        sink->mShared = source->mShared.get();
        
        std::cout << "Stage::link: Linked: " << source->mStage << ":"
        << source << " <----> " << sink->mStage <<  ":" << sink << std::endl;
        
        return true;
    }
    else {
        std::cout << "Stage::link: Source or sink already linked." << std::endl;
        return false;
    }
}

void Stage::unlink( Source *source, Sink *sink )
{
    // Only unlink if the ports are linked to each other.
    if( (source->mLinkedSink == sink) && (sink->mLinkedSource == source) ) {

        // Stop both the source and sink side stages.
        source->mStage->stop();
        sink->mStage->stop();

        sink->mShared = nullptr;
        source->mLinkedSink = nullptr;
        sink->mLinkedSource = nullptr;
        
        std::cout << "Stage::unlink: Unlinked: " << source->mStage << ":"
        << source << " <----> " << sink->mStage <<  ":" << sink << std::endl;
        
    }
    else {
        std::cout << "Stage::unlink: Source: " << source << " not linked to "
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
    
    // Resets the buffer queue.
    reset();
}

bool Stage::Source::isLinked() const {
    return !(mLinkedSink == nullptr);
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

void Stage::Source::push(std::unique_ptr<Buffer> &buffer)
{
    if( !mShared->mBufferQueue.push(buffer) ) {
        // Buffer couldn't be inserted due to the queue being full. This should
        // not happen unless the downstream sink isn't working properly.
        std::cout << "Stage::Source::push: Failed to push buffer." << std::endl;
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

bool Stage::Sink::isLinked() const {
    return !(mLinkedSource == nullptr);
}

Stage::SynchronicityMode Stage::Sink::linkSynchronicity() const {
    return mShared->mLinkSynchronicity;
}

bool Stage::Sink::checkFormatSupport( const BufferFormat &format ) const
{
    // Derp!
    return true;
}

Stage::Sink::PullResult Stage::Sink::pull( std::unique_ptr<Buffer> *outBuffer )
{
    switch(mShared->mLinkSynchronicity) {
        case kAsynchronous: {
            
            std::unique_lock<std::mutex> lock(mShared->mPushMutex);
            
            // Wait for a buffer to be pushed into the queue.
            while( mShared->mBufferQueue.empty() ) {
                mShared->mPushNotification.wait(lock);
                
                // TODO: Very sloppy to reset mPullCancelled here because other
                // threads theoretically could be waiting here.
                if( mPullCancelled ) {
                    mPullCancelled = false;
                    return kCancelled;
                }
            }
        }
        case kSynchronous: {
            // Generate a buffer only if one isn't already pending.
            if( mShared->mBufferQueue.empty() ) {
                ProcessIOFlags ioFlags = 0;
                mLinkedSource->mStage->process(&ioFlags);
            }
        }
    }
    
    if( !mShared->mBufferQueue.pop(outBuffer) ) {
        return kBufferQueueEmpty;
    }
    
    // Check if the buffer's format matches the sink's format.
    if( (*outBuffer)->format() != mBufferFormat ) {
        if( !mStage->reconfigureSink(*this, (*outBuffer)->format()) ){
            return kUnsupportedFormat;
        }
        
        // Format accepted.
        mBufferFormat = (*outBuffer)->format();
    }
    
    return kSuccess;
}

Stage::Sink::PullResult Stage::Sink::tryPull(std::unique_ptr<Buffer> *outBuffer) {
    
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
        if( !mStage->reconfigureSink(*this, (*outBuffer)->format()) ){
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
        mShared->mPushNotification.notify_all();
    }
}
