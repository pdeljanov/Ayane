/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_CLOCKPROVIDER_H_
#define AYANE_CLOCKPROVIDER_H_

#include "Ayane/Clock.h"

namespace Ayane {
    
    /**
     *  ClockCapabilties describes the capabilities of the clock provider.
     */
    typedef struct ClockCapabilities{
    public:
        
        /** The minimum clock tick period in nanoseconds. */
        uint64_t minPeriod;
        
        /** The maximum clock tick period in nanoseconds. */
        uint64_t maxPeriod;
        
        ClockCapabilities(uint64_t min, uint64_t max) :
        minPeriod(min),
        maxPeriod(max)
        {
            
        }
        
    } ClockCapabilities;
    
    /**
     *  A Clock Provider provides a source of time and an interface to
     *  register clocks to be notified of clock events.
     */
    class ClockProvider {
        
    public:

        ClockProvider(ClockCapabilities capabilties, uint64_t defaultPeriod);
        ~ClockProvider();
        
        /**
         *  Gets the clock period in nanoseconds.
         */
        uint64_t clockPeriod() const {
            return mClockPeriod;
        }
        
        /**
         *  Attempts to set the clock period to the specified number of 
         *  nanoseconds. If the clock period is valid (falls within the clock's
         *  capabilities) true will be returned, false otherwise.
         */
        bool setClockPeriod(uint64_t period);
        
        /**
         *  Gets the clock provider's capabilities.
         */
        ClockCapabilities capabilities() const {
            return mCapabilities;
        }
        
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
        AYANE_DISALLOW_COPY_AND_ASSIGN(ClockProvider);
        
        ClockCapabilities mCapabilities;
        std::list<Clock*> mSubscribers;
        
        uint64_t mClockPeriod;
    };
    
}

#endif