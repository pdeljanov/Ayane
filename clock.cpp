#include "clock.h"

using namespace Stargazer::Audio;

Clock::Clock() :
    m_started(false),
    m_currentTime(0.0f),
    m_deltaTime(0.0f),
    m_updatedTime(0.0f)
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
    
    m_updatedTime = 0.0f;
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

void Clock::advance(double time) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_updatedTime = time;
    m_cond.notify_all();
}

bool Clock::wait() {
    std::unique_lock<std::mutex> lock(m_mutex);
    
    // Only wait if the current time is the same as the current time.
    // Only wait if the clock is started.
    while( (m_updatedTime == m_currentTime) && m_started ) {
        m_cond.wait(lock);
    }
    
    // Update the times.
    m_deltaTime =  m_updatedTime - m_currentTime;
    m_currentTime = m_updatedTime;
    
    // Return clock state.
    return m_started;
}
