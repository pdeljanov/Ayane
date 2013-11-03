#include "clockprovider.h"

using namespace Stargazer::Audio;

ClockProvider::ClockProvider() {

}

ClockProvider::~ClockProvider() {
    
}

void ClockProvider::registerClock(Stargazer::Audio::Clock *clock) {
    m_subscribers.push_back(clock);
}

void ClockProvider::deregisterClock(Stargazer::Audio::Clock *clock) {
    m_subscribers.remove(clock);
}

void ClockProvider::publish(double time) {
    for (std::list<Clock*>::iterator iter = m_subscribers.begin(),
         end = m_subscribers.end(); iter != end; ++iter ) {
        
        (*iter)->advancePresentation(time);
        
    }
}
