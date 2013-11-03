#ifndef STARGAZER_STDLIB_AUDIO_CLOCKOBSERVER_H_
#define STARGAZER_STDLIB_AUDIO_CLOCKOBSERVER_H_

#include "clock.h"

namespace Stargazer {
    namespace Audio {
        
        /**
         *  A clock observer is a proxy object that forwards calls to an 
         *  underlying clock object. Clock observers are readonly in the sense
         *  that any operation that may modify the underlying clock is a no-op.
         */
        class ClockObserver : public AbstractClock {

            friend class Clock;
            
        public:
            
            ~ClockObserver();
            
            virtual double pipelineTime() const {
                return m_clock->pipelineTime();
            }
            
            virtual double presentationTime() const {
                return m_clock->presentationTime();
            }
            
            virtual double deltaTime() const {
                return m_clock->deltaTime();
            }
            
            virtual void start();
            virtual void stop();
            virtual void reset( double );
            virtual void advancePresentation( double );
            virtual void advancePipeline( double );
            
            virtual bool wait() {
                return m_clock->wait();
            }
            
        private:
            STARGAZER_DISALLOW_COPY_AND_ASSIGN(ClockObserver);
            
            ClockObserver( Clock *source );
            
            Clock *m_clock;
            
        };
        
        
    }
}

#endif