/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "clockprovider.h"

using namespace Stargazer::Audio;

ClockProvider::ClockProvider() {

}

ClockProvider::~ClockProvider() {
    
}

void ClockProvider::registerClock(Clock *clock) {
    m_subscribers.push_back(clock);
}

void ClockProvider::deregisterClock(Clock *clock) {
    m_subscribers.remove(clock);
}

void ClockProvider::publish(double time) {
    for (std::list<Clock*>::iterator iter = m_subscribers.begin(),
         end = m_subscribers.end(); iter != end; ++iter ) {
        
        (*iter)->advancePresentation(time);
        
    }
}
