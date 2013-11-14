/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

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
            void registerClock( Clock* clock );
            
            /**
             *  Cancels a clock's subscription to clock events.
             */
            void deregisterClock( Clock* clock );
            
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