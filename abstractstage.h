#ifndef STARGAZER_STDLIB_AUDIO_ABSTRACTSTAGE_H_
#define STARGAZER_STDLIB_AUDIO_ABSTRACTSTAGE_H_

#include "buffer.h"

#include <string>
#include <memory>
#include <future>
#include <unordered_map>

namespace Stargazer {
    namespace Audio {
        
        // Forward declarations.
        class SourcePort;
        class SinkPort;
        class AbstractStage;

        /**
         *  Enumeration of port availability.
         */
        typedef enum
        {
            
            /** The port is always available. **/
            kAlways = 0,
            
            /** The port is dynamically added (and removed) by the stage. **/
            kDynamic,
            
            /** The port is available upon request by the application. **/
            kOnDemand
            
        }
        PortAvailability;
        
        /**
         *  Enumeration of port link status.
         */
        typedef enum
        {
            /** The port is unlinked. **/
            kUnlinked = 0,
            
            /** The port is linked, but not negotiated. **/
            kLinked,
            
            /** The port is linked, and negotiated. **/
            kNegotiated
            
        }
        PortLinkStatus;
        
        
        /**
         *  AbstractSourceFunctor is a base class for functors given to 
         *  a source port.
         */
        class AbstractSourceFunctor
        {
        public:
            virtual Buffer *operator()( const AbstractStage &owner ) = 0;
        };
        
        /**
         *  Ports is a utility class for working with pairs of ports.
         */
        class Ports
        {
        public:
            
            static bool link(std::shared_ptr<SourcePort> &source,
                             std::shared_ptr<SinkPort> &sink );
            
            static void unlink(std::shared_ptr<SourcePort> &source,
                               std::shared_ptr<SinkPort> &sink );
        };
        
        /**
         *  A source port is a producer of audio data.
         */
        class SourcePort
        {
            friend class Ports;
            
        public:
            
            /** Enumeration of synchronicity modes for a Source Port. Synchronicity modes
             *  control how a stage will be threaded, and when port calls will be fulfilled.
             */
            typedef enum
            {
                /** Asynchronous operating mode. The port will request that the stage
                 *  executes in a new thread separate from the port caller. Futures 
                 *  promised by the port will be fulfilled by the stage's processing thread.
                 */
                kAsynchronous,
                /** Synchronous operating mode. The port will request the stage executes
                 *  in the same thread as the port caller. Futures promised by the port will
                 *  be fulfilled immediately.
                 */
                kSynchronous
            }
            SynchronicityMode;
            

            /**
             *  Creates a new instance of SourcePort for the given stage.
             */
            SourcePort( const AbstractStage &stage );
            
            /** Pushes a processed buffer to the port. If there are pending 
             *  pull requests, they will be fulfilled first before the buffer
             *  if queued. */
            void push();
            
            /**
             *  Attempts to renegotiate the buffer format. Returns true if the 
             *  linked sink will accept the new format, false otherwise. If 
             *  negotiation is succesfull, the owning stage will be reconfigured
             *  to output the new format.
             */
            bool tryNegotiateFormat( const BufferFormat &format );
            
            /**
             *  Requeusts a buffer future.
             */
            std::future< std::shared_ptr<Buffer> > request();
            
            /**
             *  Gets the synchonicity mode. Only valid for a linked port.
             */
            SynchronicityMode synchronicity() const {
                return m_synchronicity; }
            
        private:
            
            const AbstractStage &m_stage;
            SynchronicityMode m_synchronicity;
            
            std::weak_ptr<SinkPort> m_sink;
        };
        
        /**
         *  A sink port is a consumer of audio data.
         */
        class SinkPort
        {
            friend class Ports;
            
        public:
            
            /** Enumeration of scheduling modes for a Sink Port. Scheduling modes help
             *  the pipeline assign a synchronicity mode to a source port and in turn, 
             *  affects the pipeline execution.
             */
            typedef enum
            {
                /**
                 *  Default scheduling. No hints to the pipeline scheduler. This mode is guaranteed
                 *  to be safe for all valid pipeline use cases.
                 */
                kDefault,
                /**
                 *  Forces the upstream port to operating in asynchronous mode. This mode can be helpful
                 *  when interfacing to outputs running on their own thread (such as OS audio devices).
                 *  This mode is safe, but may not be as performant since it will create excess processing
                 *  threads. Therefore, only use when needed.
                 */
                kForceAsynchronous
                
            }
            SchedulingMode;
            
            SinkPort( const AbstractStage &stage );
            
            /**
             *  Attempts to negotiate the buffer format. Upon successful negotiation,
             *  all current receive requests will be cancelled, and the owner stage
             *  will be reconfigured to support the new format.
             */
            bool tryNegotiateFormat( const BufferFormat &format );
            
            /**
             *  Attempts to receive a buffer future. Depending on the upstream synchronicity mode,
             *  calling get() on the received future may block. Calling receive multiple times before
             *  the future has been fulfilled will result in returning the same future.
             */
             std::future<std::shared_ptr<Buffer>> receive();
            
            /**
             *  Gets the scheduling mode.
             */
            SchedulingMode scheduling() const {
                return m_scheduling;
            }
            
            /**
             *  Sets the scheduling mode. Only valid before a port is linked.
             */
            bool setScheduling(SchedulingMode mode);
            
        private:
            
            const AbstractStage &m_stage;
            SchedulingMode m_scheduling;
            std::weak_ptr<SourcePort> m_source;

        };
        
        
        
        
        
        /**
         *  A Port represents an input or output for a stage.
         */
        class AbstractStage
        {

        public:
            
            /** Collection type for source ports. */
            typedef std::unordered_map<std::string, std::shared_ptr<SourcePort>> SourceCollection;
            
            /** Collection type for sink ports. */
            typedef std::unordered_map<std::string, std::shared_ptr<SinkPort>> SinkCollection;

            typedef SourceCollection::iterator SourceIterator;
            typedef SourceCollection::const_iterator ConstSourceIterator;
            
            typedef SinkCollection::iterator SinkIterator;
            typedef SinkCollection::const_iterator ConstSinkIterator;
            
            typedef std::pair<SourceIterator, SourceIterator> SourceIteratorPair;
            typedef std::pair<ConstSourceIterator, ConstSourceIterator> ConstSourceIteratorPair;
            
            typedef std::pair<SinkIterator, SinkIterator> SinkIteratorPair;
            typedef std::pair<ConstSinkIterator, ConstSinkIterator> ConstSinkIteratorPair;
            
            

            /* Source Interface */
            
            /** Retreives a source port by its name. */
            bool sourcePort( const std::string &name, std::weak_ptr<SourcePort> *port );
            
            /** Gets an interator pair (begin, and end) for the source ports. */
            ConstSourceIteratorPair sourcePortsIterator(){
                return std::make_pair<ConstSourceIterator, ConstSourceIterator>(m_sources.begin(), m_sources.end());
            }
            
            /** Gets the number of sources supported by the stage. */
            int numberOfSources() const {
                return m_sources.size();
            }
            
            
            /* Sink Interface */
            
            /** Retreives a sink port by its name. */
            bool sinkPort( const std::string &name, std::weak_ptr<SinkPort> *port );
            
            /** Gets the number of sinks supported by the stage. */
            int numberOfSinks() const {
                return m_sinks.size();
            }

            /** Gets an iterator pair (begin and end) for the sink ports. */
            ConstSinkIteratorPair sinkPortsIterator(){
                return std::make_pair<ConstSinkIterator, ConstSinkIterator>(m_sinks.begin(), m_sinks.end());
            }



            

        protected:
            
            void addSourcePort( const std::string &name, SourcePort *port );
            void addSinkPort( const std::string &name, SinkPort *port );
            
            SourceCollection m_sources;
            SinkCollection m_sinks;
            
        };
        
        
        
        
    }
}

#endif
