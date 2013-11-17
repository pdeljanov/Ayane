/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "clockobserver.h"
#include "trace.h"

using namespace Stargazer::Audio;

ClockObserver::ClockObserver( Clock *source ) : mClock(source) {
    
}

ClockObserver::~ClockObserver() {
    mClock->removeObserver(this);
}

void ClockObserver::start() {
    // ClockObserver is readonly.
}

void ClockObserver::stop() {
    // ClockObserver is readonly.
}

void ClockObserver::reset(double) {
    // ClockObserver is readonly.
}

void ClockObserver::advancePresentation(double) {
    NOTICE_THIS("ClockObserver::advancePresentation")
    << "Attempted to advance a clock observer." << std::endl;
}

void ClockObserver::advancePipeline(double) {
    NOTICE_THIS("ClockObserver::advancePipeline")
    << "Attempted to advance a clock observer." << std::endl;
}
