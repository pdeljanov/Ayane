#ifndef STARGAZER_STDLIB_AUDIO_ABSTRACTSTAGE_H_
#define STARGAZER_STDLIB_AUDIO_ABSTRACTSTAGE_H_

#include "buffer.h"
#include "clock.h"

#include <memory>
#include <mutex>
#include <thread>
#include <string>
#include <queue>
#include <unordered_map>

namespace Stargazer {
    namespace Audio {
        
        /**
         *  A Port represents an input or output for a stage.
         */
        class Stage {
        public:
            
            class Sink;

            /**
             *  A source provides the producing side of a one-to-one
             *  connection between stages. When linked to a sink, a source
             *  provides an interface for buffers to be pushed from the source
             *  stage, and pulled from the sink stage.
             */
            class Source {
                
                friend class Stage;
                
            public:
                
                /** Enumeration of synchronicity modes for a Stage. Synchronicity modes
                 *  control how a stage will be threaded.
                 */
                typedef enum
                {
                    
                    /** Asynchronous operating mode. The Stage pushes buffers to its
                     *  source ports in parallel with pull requests.
                     */
                    kAsynchronous,
                    
                    /** Synchronous operating mode. The Stage pushes buffers to its
                     *  source ports only when a pull request is made.
                     */
                    kSynchronous
                    
                }
                SynchronicityMode;
                
                /**
                 *  Creates a new instance of Source.
                 */
                Source( Stage &stage );
                
                /**
                 *  Source destructor. Clears all pending buffers and cancels
                 *  any waiting pulls.
                 */
                ~Source();
                
                /**
                 *  Attempts to push buffer to the source. If the source buffer
                 *  queue is full, this function will block.
                 */
                void push( std::shared_ptr<Buffer> &buffer );
                
                /**
                 *  Pulls a buffer from the source. This function blocks until a
                 *  buffer is pulled.
                 */
                bool pull( std::shared_ptr<Buffer> &buffer );
                
                /**
                 *  Attempts to pull a buffer from the source. This function 
                 *  never blocks, but a buffer may not always be pulled.
                 */
                bool tryPull( std::shared_ptr<Buffer> &buffer );

                /** 
                 *  Cancels any pending request on the source port. 
                 */
                void cancel();
                
                /**
                 *  Resets the source by clearing all pending buffers.
                 */
                void reset();
                
                /**
                 *  Asks the Sink if it supports buffers of the specified format.
                 */
                bool checkFormatSupport( const BufferFormat &format ) const;
                
                /**
                 *  Gets the synchonicity mode. Only valid after the stage is
                 *  activated.
                 */
                SynchronicityMode synchronicity() const {
                    return m_synchronicity;
                }
                
            private:
                
                Stage &m_stage;
                
                // Queue of processed buffers.
                std::queue<std::shared_ptr<Buffer>> m_buffers;
                
                // Mutex, and condition variables to implement blocking.
                std::mutex m_mutex;
                std::condition_variable m_cv;
                volatile bool m_cancelled;
                
                // Synchronicity operating mode.
                SynchronicityMode m_synchronicity;

                
                Sink *m_sink;
            };
            
            /**
             *  A sink provides the consuming side of a one-to-one connection
             *  between stages.  When linked to a source, a sink provides the 
             *  interface to pull buffers from the linked source's stage.
             */
            class Sink {
                
                friend class Stage;
                
            public:
                
                /** Enumeration of scheduling modes for a Sink Port. Scheduling 
                 *  modes help the pipeline assign a synchronicity mode to the 
                 *  stage belonging to the sink's linked source port.
                 */
                typedef enum
                {
                    /**
                     *  Default scheduling. No hints to the pipeline scheduler. 
                     *  This mode is guaranteed to be safe for all valid 
                     *  pipeline use cases.
                     */
                    kDefault,
                    /**
                     *  Forces the upstream port's stage to operate in 
                     *  asynchronous mode. This mode can be helpful when 
                     *  interfacing to outputs running on their own thread 
                     *  (such as OS audio devices). This mode is safe, but may 
                     *  not be as performant since it will create excess 
                     *  processing threads. Therefore, only use when needed.
                     */
                    kForceAsynchronous
                    
                }
                SchedulingMode;

                /**
                 *  Creates a new instance of Sink.
                 */
                Sink( Stage &stage );

                /**
                 *  Tests if the buffer format is compatible with the sink.
                 */
                bool checkFormatSupport( const BufferFormat &format ) const;
                
                /**
                 *  Requests a buffer from the linked source. This function will
                 *  wait until the source services the request. The returned buffer
                 *  may be a different format between successive calls, but the sink
                 *  will issue a port-specific reconfigureSink() event.
                 */
                std::shared_ptr<Buffer> pull();
                
                /**
                 *  Attempts to pull a buffer from the linked source. This 
                 *  function will never block, but it may not always return a 
                 *  buffer.
                 */
                bool tryPull( std::shared_ptr<Buffer> &buffer );
                
                /**
                 *  Gets the scheduling mode.
                 */
                SchedulingMode scheduling() const {
                    return m_scheduling;
                }
                
                /**
                 *  Sets the scheduling mode. Only valid before a port is linked.
                 */
                void setScheduling(SchedulingMode mode) {
                    m_scheduling = mode;
                }
                
            private:

                Stage &m_stage;
                SchedulingMode m_scheduling;
                
                Source *m_source;
            };
            
            
            /** Collection type for sources. */
            typedef std::unordered_map<std::string, Source*> SourceCollection;
            
            /** Collection type for sinks. */
            typedef std::unordered_map<std::string, Sink*> SinkCollection;

            typedef SourceCollection::iterator SourceIterator;
            typedef SourceCollection::const_iterator ConstSourceIterator;
            
            typedef SinkCollection::iterator SinkIterator;
            typedef SinkCollection::const_iterator ConstSinkIterator;
            
            typedef std::pair<SourceIterator, SourceIterator> SourceIteratorPair;
            typedef std::pair<ConstSourceIterator, ConstSourceIterator> ConstSourceIteratorPair;
            
            typedef std::pair<SinkIterator, SinkIterator> SinkIteratorPair;
            typedef std::pair<ConstSinkIterator, ConstSinkIterator> ConstSinkIteratorPair;



            /** Enumeration of Stage operating states. */
            typedef enum
            {
                
                /** The Stage is not initialized or capable of playing. */
                kDeactivated,
                
                /** The Stage is initialized, and capable of playing. */
                kActivated,
                
                /** The Stage is playing. */
                kPlaying
                
            } State;

            
            
            Stage();
            virtual ~Stage();
            
            /**
             *  Attempts to retreive a source port by name.
             */
            bool source( const std::string &name, Source *port );
            
            /**
             *  Attempts to retreive a sink by name.
             */
            bool sink( const std::string &name, Sink *port );
            
            /** 
             *  Gets an iterator begin/end pair for sources.
             */
            ConstSourceIteratorPair sourceIterator(){
                return std::make_pair( m_sources.begin(), m_sources.end() );
            }
            
            /**
             *  Gets an iterator begin/end pair for sinks.
             */
            ConstSinkIteratorPair sinkIterator(){
                return std::make_pair( m_sinks.begin(), m_sinks.end() );
            }
            
            /**
             *  Gets the number of sources.
             */
            int sourceCount() const {
                return m_sources.size();
            }
            
            /**
             *  Gets the number of sinks.
             */
            int sinkCount() const {
                return m_sinks.size();
            }
            
            


            
            /** 
             *  Activates the stage to prepare it for playback. Thread-safe. 
             */
            bool activate();
            
            /** 
             *  Deactivates the stage. Thread-safe. 
             */
            void deactivate();

            /** 
             *  Starts playback. Once called, the stage will produce buffers
             *  clocked by the clock provider. Thread-safe.
             */
            void play( AbstractClock *clock );
            
            /** 
             *  Stops playback. Thread-safe. 
             */
            void stop();
            


            /**
             *  Gets the stage's state.
             */
            State state() const {
                return m_state;
            }
            
            
            /** 
             *  Called by the Stage when the next set of buffers should be
             *  pushed to the Stage's sources.
             */
            virtual void process() = 0;

            /**
             *  Called by the Stage when the buffer format of sink changes.
             *
             *  This function is always called on the processing thread after
             *  a pull call is made on a sink, but before the buffer is returned
             *  to the caller.
             *
             */
            virtual bool reconfigureSink( const Sink &sink ) = 0;
            

            /**
             *  Links the specified source and sink together.
             */
            static bool link( Source *source, Sink *sink );
            
            /**
             *  Unlinks the specified source from the sink.
             */
            static void unlink( Source *source, Sink *sink );
            
        protected:

            void addSource( const std::string &name, Source *port );
            void removeSource( const std::string &name );

            void addSink( const std::string &name, Sink *port );
            void removeSink( const std::string &name );
            
            /**
             *  Gets the stage clock.
             */
            AbstractClock *clock() const {
                return m_clock;
            }
            
            /** Called by the Stage when transitioning from Activated to
             *  Playing.
             *
             *  The state of the stage between invocations of beginPlayback is
             *  not guaranteed to be the same. Therefore, beginPlayback is the
             *  ideal callback to probe the stage to determine any necessary
             *  processing parameters.
             */
            virtual bool beginPlayback() = 0;
            
            /** Called by the Stage when transitioning from either Paused
             *  or Playing to Stopped.
             */
            virtual bool stoppedPlayback() = 0;
            
            
            SourceCollection m_sources;
            SinkCollection m_sinks;
            
        private:
            

            /** Begins asynchronous processing. */
            void startAsyncProcess();
            
            /** Stops asynchronous processing. */
            void stopAsyncProcess();
            
            /** Asynchronous processing loop. */
            void asyncProcessLoop();
            
            /** Stop function without locking. */
            void stopNoLock();
            
            /** 
             *  Determines if the stage should opeate asynchronously given the
             *  current configuration.
             */
            bool shouldRunAsynchronous() const;
            
            // State mutex.
            std::mutex m_mutex;
            State m_state;
            
            // Thread for asynchronous processing.
            std::thread m_thread;
            AbstractClock *m_clock;
            bool m_processingAsync;

        };
        
        
        
        
    }
}

#endif
