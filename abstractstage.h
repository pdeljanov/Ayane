#ifndef STARGAZER_STDLIB_AUDIO_ABSTRACTSTAGE_H_
#define STARGAZER_STDLIB_AUDIO_ABSTRACTSTAGE_H_

#include "buffer.h"

#include <memory>
#include <future>
#include <vector>

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
            
            const std::vector< std::shared_ptr<SourcePort> > &sources() const {
                return m_sources;
            };
            
            const std::vector< std::shared_ptr<SinkPort> > &sinks() const {
                return m_sinks;
            }

        protected:
            std::vector< std::shared_ptr<SourcePort> > m_sources;
            std::vector< std::shared_ptr<SinkPort> > m_sinks;
            
        };
        
        
        
        
    }
}

#endif
