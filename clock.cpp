/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "clock.h"
#include "clockobserver.h"

using namespace Stargazer::Audio;

Clock::Clock() :
    mStarted(false),
    mPipelineTime(0.0),
    mPresentationTime(0.0),
    mDeltaTime(0.0),
    mUpdateDelta(0.0)
{
    
}

Clock::~Clock() {
    stop();
}

void Clock::start() {
    std::lock_guard<std::mutex> lock(mStateMutex);
    
    if( mStarted ) {
        return;
    }
    
    //m_updateDelta = 0.0;
    mStarted = true;
    mAdvanceNotification.notify_all();
}

void Clock::stop() {
    std::lock_guard<std::mutex> lock(mStateMutex);

    if( !mStarted ) {
        return;
    }

    mStarted = false;
    mAdvanceNotification.notify_all();
}

void Clock::reset( double time ) {
    std::unique_lock<std::mutex> lock(mStateMutex);
    mUpdateDelta = time - mPresentationTime;
    mAdvanceNotification.notify_all();
}

void Clock::advancePresentation(double delta) {
    std::unique_lock<std::mutex> lock(mStateMutex);

    mUpdateDelta = delta;
    mAdvanceNotification.notify_all();
}

void Clock::advancePipeline(double delta) {
    mPipelineTime += delta;
}

bool Clock::wait() {
    std::unique_lock<std::mutex> lock(mStateMutex);
    
    // Only wait if the current time is the same as the current time.
    // Only wait if the clock is started.
    while( (mUpdateDelta == 0.0) && mStarted ) {
        mAdvanceNotification.wait(lock);
    }
    
    // Update the times.
    mDeltaTime = mUpdateDelta;
    mPresentationTime += mUpdateDelta;
    
    // Reset update delta.
    mUpdateDelta = 0.0;
    
    // Return clock state.
    return mStarted;
}

ClockObserver *Clock::makeObserver() {
    ClockObserver *observer = new ClockObserver(this);
    mObservers.push_back(observer);
    return observer;
}

void Clock::removeObserver(ClockObserver *observer) {
    mObservers.remove(observer);
}
