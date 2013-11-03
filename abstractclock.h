#ifndef STARGAZER_STDLIB_AUDIO_ABSTRACTCLOCK_H_
#define STARGAZER_STDLIB_AUDIO_ABSTRACTCLOCK_H_

#include <core/macros.h>

namespace Stargazer {
    namespace Audio {
        
        /**
         *  Pure virtual base class for a clock implementation.
         */
        class AbstractClock {
            
        public:
            
            /**
             *  Gets the current timestamp of the pipeline.
             */
            virtual double pipelineTime() const = 0;
            
            /**
             *  Gets the current output (playback) timestamp.
             */
            virtual double presentationTime() const = 0;
            
            /**
             *  Gets the time delta between the last two successive wait()
             *  calls.
             */
            virtual double deltaTime() const = 0;

            /**
             *  Starts the clock. Resets the running time.
             */
            virtual void start() = 0;
            
            /**
             *  Stops the clock.
             */
            virtual void stop() = 0;
            
            /**
             *  Resets the clock to the specified time.
             */
            virtual void reset( double time = 0.0 ) = 0;
            
            /**
             *  Advances the presentation clock by the specified time delta. 
             *  All threads that are blocked on a wait() will be unblocked.
             */
            virtual void advancePresentation( double time ) = 0;
            
            /**
             *  Advances the pipeline clock by the specified time delta.
             */
            virtual void advancePipeline( double delta ) = 0;
            
            /**
             *  Waits for the clock to advance. Returns true if the clock is
             *  running, or false if it was stopped.
             */
            virtual bool wait() = 0;
            
        };
        
        
    }
}

#endif