#ifndef STARGAZER_STDLIB_AUDIO_CLOCKPROVIDER_H_
#define STARGAZER_STDLIB_AUDIO_CLOCKPROVIDER_H_

#include "clock.h"

namespace Stargazer {
    namespace Audio {
        
        
        /**
         *  A Clock Provider provides a source of time and an interface to
         *  register clocks to be notified of clock events.
         */
        class ClockProvider {
            
        public:
            
            ClockProvider();
            ~ClockProvider();
            
            /**
             *  Registers a clock to be notified of clock events.
             */
            void registerClock( Clock* );
            
            /**
             *  Cancels a clock's subscription to clock events.
             */
            void deregisterClock( Clock* );
            
            /**
             *  Publishes a clock event to the provider's subscribers.
             */
            void publish( double time );
            
        private:
            STARGAZER_DISALLOW_COPY_AND_ASSIGN(ClockProvider);
            
            std::list<Clock*> m_subscribers;
            
        };
        
        
    }
}

#endif