#include "abstractstage.h"

#include <iostream>

using namespace Stargazer::Audio;

bool Ports::link( std::shared_ptr<SourcePort> &source, std::shared_ptr<SinkPort> &sink )
{
    /* Synchonicity mode
     *
     * The synchonicity mode of the *source* port is determined by the graph relationships
     * of both ports, and the scheduling mode of the sink port.
     *
     *                       |  # of Sinks (receiver)
     *  ---------------------+-----------+------------
     *                       |    One    |   Many
     *  ----------------------------------------------
     *  # of Sources | One   |   Sync    |   Async
     *    (sender)   | Many  |   Async   |   Async
     *
     * However, if the sink is in ForceAsynchonous mode, the source will always operate
     * asynchonously.
     *
     */

    if( sink->scheduling() == SinkPort::kForceAsynchronous ) {
        source->m_synchronicity = SourcePort::kAsynchronous;
    }
    else
    {
        // Number of sources on the sender (i.e., the source port)
        int numSources = source->m_stage.numberOfSources();
        // Number of sinks on the receiver (i.e., the sink port)
        int numSinks = sink->m_stage.numberOfSinks();

        // TODO: assert( (numSources > 0) && (numSinks > 0) );
        
        if( (numSources == 1) && (numSinks == 1) ) {
            source->m_synchronicity = SourcePort::kSynchronous;
        }
        else {
            source->m_synchronicity = SourcePort::kAsynchronous;
        }
    }
    
    // Link the two ports (-> weak_ptr conversion).
    source->m_sink = sink;
    sink->m_source = source;
    
    std::cout << "Ports::link: Linked: " << sink.get() << " <----> "
        << source.get() << " with source SynchronicityMode = "
        << ((source->m_synchronicity == SourcePort::kSynchronous) ? "Sync" : "Async")
        << std::endl;
    
    return true;
}

void Ports::unlink( std::shared_ptr<SourcePort> &source, std::shared_ptr<SinkPort> &sink )
{
    std::cout << "Ports::unlink: Unlinking: " << sink.get() << " <-  -> "
        << source.get() << std::endl;
    
    // TODO: Verify the two ports are linked to one another.
    
    source->m_sink.reset();
    sink->m_source.reset();
}

SourcePort::SourcePort( const AbstractStage &stage ) :
    m_stage(stage),
    m_synchronicity(kAsynchronous)
{
    
}




SinkPort::SinkPort( const AbstractStage &stage ) :
    m_stage(stage)
{
    
}


void AbstractStage::addSourcePort(const std::string &name, SourcePort *port)
{
    m_sources.insert( std::make_pair(name, std::shared_ptr<SourcePort>(port)) );
}

void AbstractStage::addSinkPort(const std::string &name, SinkPort *port)
{
    m_sinks.insert( std::make_pair(name, std::shared_ptr<SinkPort>(port)) );
}