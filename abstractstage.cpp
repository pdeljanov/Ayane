#include "abstractstage.h"

#include <iostream>

using namespace Stargazer::Audio;

/* Stage */

Stage::Stage() : m_state(kDeactivated), m_clock(nullptr), m_processingAsync(false)
{
    
}

Stage::~Stage()
{
    // Stage was "killed"
    deactivate();
}

void Stage::asyncProcessLoop()
{
    while(m_clock->wait()) {
        process();
    }

    std::cout << "Stage::asyncProcessLoop: Processing thread with ID: "
    << m_thread.get_id() << " done." << std::endl;
}

void Stage::startAsyncProcess()
{
    if( !m_thread.joinable() ){

        // Start the clock.
        m_clock->start();
        m_thread = std::thread( &Stage::asyncProcessLoop, this );
        
        std::cout << "Stage::startAsyncProcess: Started thread with ID: "
        << m_thread.get_id() << "." << std::endl;
    }
    else
    {
        std::cout << "Stage::startAsyncProcess: Thread already started with ID: "
        << m_thread.get_id() << "." << std::endl;
    }

}

void Stage::stopAsyncProcess()
{
    if( m_thread.joinable() ){
        
        std::cout << "Stage::stopAsyncProcess: Waiting for processing thread to"
        " stop." << std::endl;
        
        // Stop the clock. Processing thread will exit
        m_clock->stop();
        m_thread.join();
        
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
        for(ConstSourceIterator iter = m_sources.begin(), end = m_sources.end();
            iter != end; ++iter)
        {
            const Sink *sink = iter->second->m_sink;
            
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
            if( sink->m_stage.sinkCount() > 1 ) {
                return true;
            }
        }
    }
    
    return false;
}


bool Stage::activate()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Deactivated -> Activated.
    if( m_state == kDeactivated ) {
        
        // Switch state to activated.
        m_state = kActivated;
        
        std::cout << "Stage::activate: Stage (" << this << ") activated."
        << std::endl;
        
        return true;
    }
    
    return false;
}

void Stage::deactivate()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // If the stage is in the playing state, stop playback.
    if( m_state == kPlaying ) {
        // Stop playback. Use no-lock variant since we have the state lock.
        stopNoLock();
    }
    
    // If the stage is activated, proceed to deactivate it.
    if( m_state == kActivated ) {

        // Reset sources (clears synchronicity and empties buffers).
        for(SourceIterator iter = m_sources.begin(), end = m_sources.end();
            iter != end; ++iter)
        {
            iter->second->reset();
        }
        
        // Record state.
        m_state = kDeactivated;
        
        std::cout << "Stage::deactivate: Stage (" << this << ") deactivated."
        << std::endl;
    }
}

void Stage::play( AbstractClock *clock )
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // Activated (Stopped) -> Playing
    if( m_state == kActivated ) {
        
        // Set the clock.
        m_clock = clock;
        
        // Determine synchronicity.
        m_processingAsync = shouldRunAsynchronous();
        
        std::cout << "Stage::play: Stage (" << this << ") will run "
        << (m_processingAsync ? "asynchronously." : "synchronously.")
        << std::endl;
        
        // Assign synchronicity mode to the sources.
        Source::SynchronicityMode mode =
            (m_processingAsync) ? Source::kAsynchronous : Source::kSynchronous;
        
        for(SourceIterator iter = m_sources.begin(), end = m_sources.end();
            iter != end; ++iter)
        {
            iter->second->m_synchronicity = mode;
        }

        // Begin playback callback. Must occur before buffers are processed!
        beginPlayback();
 
        if( m_processingAsync ) {
            // If asynchronous, start the processing thread.
            startAsyncProcess();
        }
        else {
            // Start the clock manually in synchronous mode.
            m_clock->start();
        }
        
        // Record the state.
        m_state = kPlaying;
        
        std::cout << "Stage::play: Stage (" << this << ") playing."
        << std::endl;
    }
    
}

void Stage::stop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    stopNoLock();
}

void Stage::stopNoLock()
{
    if( m_state == kPlaying ) {
        
        if( m_processingAsync ) {
            // If asynchronous, stop the processing thread.
            // Wait till processing stops.
            stopAsyncProcess();
        }
        else {
            // If synchronous, stop the clock manually.
            m_clock->stop();
        }
        
        // Playback stopped callback. Must occur after all buffers are
        // processed.
        stoppedPlayback();

        // Record the state.
        m_state = kActivated;
    }
}



void Stage::addSource(const std::string &name, Source *port)
{
    if( m_state == kDeactivated ){
        m_sources.insert( std::make_pair(name, port) );
    }
    else {
        std::cout << "Stage::addSource: Can't add source unless stage is "
        "deactivated." << std::endl;
    }
}

void Stage::addSink(const std::string &name, Sink *port)
{
    if( m_state == kDeactivated ) {
        m_sinks.insert( std::make_pair(name, port) );
    }
    else {
        std::cout << "Stage::addSink: Can't add sink unless stage is "
        "deactivated." << std::endl;
    }
}


bool Stage::link( Source *source, Sink *sink )
{
    if( (source->m_sink == nullptr) && (sink->m_source == nullptr) ) {
        
        source->m_sink = sink;
        sink->m_source = source;
    
        std::cout << "Stage::link: Linked: " << sink << " <----> "
        << source << std::endl;
        
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
    if( (source->m_sink == sink) && (sink->m_source == source) ) {

        // Stop both the source and sink side stages.
        source->m_stage.stop();
        sink->m_stage.stop();
        
        source->m_sink = nullptr;
        sink->m_source = nullptr;
        
        std::cout << "Stage::unlink: Unlinked: " << sink << " <-/ /-> " << source
        << std::endl;
    }
    else {
        std::cout << "Stage::unlink: Source: " << source << " not linked to "
        "sink: " << sink << std::endl;
    }
}


/* Stage::Source */

Stage::Source::Source( Stage &stage ) :
    m_stage(stage),
    m_cancelled(false),
    m_synchronicity(kSynchronous)
{
    
}

Stage::Source::~Source() {
    
    // Cancels any awaiting pulls.
    cancel();
    
    // Resets the buffer queue.
    reset();
}

bool Stage::Source::checkFormatSupport(const BufferFormat &format) const
{
    if( !m_sink->checkFormatSupport(format) ) {
        return true;
    }
    
    return false;
}

void Stage::Source::push(std::shared_ptr<Buffer> &buffer)
{
    switch(m_synchronicity) {
        case kAsynchronous: {
            
            std::lock_guard<std::mutex> lock(m_mutex);

            // Insert null shared_ptr into the queue and then swap in buffer to
            // prevent copy overhead.
            m_buffers.emplace();
            m_buffers.back().swap(buffer);

            // Notify any waiting pulls.
            m_cv.notify_one();
            
            break;
        }
        case kSynchronous: {
            
            // Insert null shared_ptr into the queue and then swap in buffer to
            // prevent copy overhead.
            m_buffers.emplace();
            m_buffers.back().swap(buffer);
            
            break;
        }
    }
}

void Stage::Source::cancel()
{
    // No-op in synchronous mode.
    if( m_synchronicity == kAsynchronous ) {
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Set cancellation flag.
        m_cancelled = true;
        
        // Notify any waiting pulls that it can cancel its wait.
        m_cv.notify_one();
        
    }
}

bool Stage::Source::pull( std::shared_ptr<Buffer> &buffer )
{
    switch(m_synchronicity) {
        case kAsynchronous: {
            
            std::unique_lock<std::mutex> lock(m_mutex);
            
            // Wait for a buffer to be pushed into the queue.
            while( m_buffers.empty() ) {
                m_cv.wait(lock);

                if( m_cancelled ) {
                    m_cancelled = false;
                    return false;
                }
            }
            
            // Return the popped buffer.
            buffer.swap(m_buffers.front());
            m_buffers.pop();
            
            return true;
        }
        case kSynchronous: {

            // Generate a buffer only if one isn't already pending.
            if( m_buffers.empty() ) {
                m_stage.process();
            }
            
            // Return the popped buffer.
            buffer.swap(m_buffers.front());
            m_buffers.pop();
            
            return true;
        }
    }
    
}

void Stage::Source::reset() {
    // Clear the queue.
    std::queue<std::shared_ptr<Buffer>>().swap(m_buffers);
}

/* Stage::Sink */

Stage::Sink::Sink( Stage &stage ) :
    m_stage(stage)
{
    
}

bool Stage::Sink::checkFormatSupport( const BufferFormat &format ) const
{
    // Derp!
    return true;
}

std::shared_ptr<Buffer> Stage::Sink::pull()
{
    std::shared_ptr<Buffer> buffer;
    
    // TODO: Test for a pull error.
    m_source->pull(buffer);
    
    // If the buffer has a format negotiation flag, issue a reconfigure
    // call which will allow the stage to adapt for the new format.
    if( buffer->flags() & Buffer::kFormatNegotiation ) {
        m_stage.reconfigureSink( *this );
    }
    
    return buffer;
}
