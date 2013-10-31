#ifndef STARGAZER_STDLIB_AUDIO_CLOCK_H_
#define STARGAZER_STDLIB_AUDIO_CLOCK_H_

#include <core/macros.h>
#include <mutex>
#include <condition_variable>

namespace Stargazer {
    namespace Audio {
        
        class Clock {
            
        public:

            Clock();
            ~Clock();
            
            /**
             *  Gets the running time of the clock.
             */
            double time() const {
                return m_currentTime;
            }
            
            /**
             *  Gets the time delta between the last two successive wait() 
             *  calls.
             */
            double deltaTime() const {
                return m_deltaTime;
            }
            
            /**
             *  Starts the clock. Resets the running time.
             */
            void start();
            
            /**
             *  Stops the clock.
             */
            void stop();
            
            /** 
             *  Advances the clock by the specified time delta. All threads that
             *  are blocked on a wait() will be unblocked.
             */
            void advance( double delta );
            
            /**
             *  Waits for the clock to advance. Returns true if the clock is 
             *  running, or false if it was stopped.
             */
            bool wait();
            
        private:
            STARGAZER_DISALLOW_COPY_AND_ASSIGN(Clock);
            
            bool m_started;
            
            double m_currentTime;
            double m_deltaTime;
            double m_updatedTime;
            
            std::mutex m_mutex;
            std::condition_variable m_cond;
            
        };
        
    }
}

#endif