/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "Ayane/ClockProvider.h"

using namespace Ayane;

ClockProvider::ClockProvider(ClockCapabilities capabilities, uint64_t defaultPeriod) :
mCapabilities(capabilities),
mClockPeriod(defaultPeriod)
{

}

ClockProvider::~ClockProvider() {
    
}

bool ClockProvider::setClockPeriod(uint64_t period) {

    if((period <= mCapabilities.maxPeriod) &&
       (period >= mCapabilities.minPeriod))
    {
        mClockPeriod = period;
        return true;
    }
    else {
        return false;
    }

}

void ClockProvider::registerClock(Clock *clock) {
    mSubscribers.push_back(clock);
}

void ClockProvider::deregisterClock(Clock *clock) {
    mSubscribers.remove(clock);
}

void ClockProvider::publish(double time) {
    for (std::list<Clock*>::iterator iter = mSubscribers.begin(),
         end = mSubscribers.end(); iter != end; ++iter ) {
        
        (*iter)->advancePresentation(time);
        
    }
}
