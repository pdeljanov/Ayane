#include "clock.h"
#include "clockobserver.h"

#include <iostream>

using namespace Stargazer::Audio;

Clock::Clock() :
    m_started(false),
    m_pipelineTime(0.0),
    m_presentationTime(0.0),
    m_deltaTime(0.0),
    m_updateDelta(0.0)
{
    
}

Clock::~Clock() {
    stop();
}

void Clock::start() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if( m_started ) {
        return;
    }
    
    m_updateDelta = 0.0;
    m_started = true;
}

void Clock::stop() {
    std::lock_guard<std::mutex> lock(m_mutex);

    if( !m_started ) {
        return;
    }

    m_started = false;
    m_cond.notify_all();
}

void Clock::reset( double time ) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_updateDelta = time - m_presentationTime;
    m_cond.notify_all();
}

void Clock::advancePresentation(double delta) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_updateDelta = delta;
    m_cond.notify_all();
}

void Clock::advancePipeline(double delta) {
    m_pipelineTime += delta;
}

bool Clock::wait() {
    std::unique_lock<std::mutex> lock(m_mutex);
    
    // Only wait if the current time is the same as the current time.
    // Only wait if the clock is started.
    while( (m_updateDelta == 0.0) && m_started ) {
        m_cond.wait(lock);
    }
    
    // Update the times.
    m_deltaTime = m_updateDelta;
    m_presentationTime += m_updateDelta;
    
    // Reset update delta.
    m_updateDelta = 0.0;
    
    // Return clock state.
    return m_started;
}

ClockObserver *Clock::makeObserver() {
    ClockObserver *observer = new ClockObserver(this);
    m_observers.push_back(observer);
    return observer;
}

void Clock::removeObserver(ClockObserver *observer) {
    m_observers.remove(observer);
}
