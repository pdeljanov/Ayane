/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "clockprovider.h"

using namespace Ayane;

ClockProvider::ClockProvider() {

}

ClockProvider::~ClockProvider() {
    
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
