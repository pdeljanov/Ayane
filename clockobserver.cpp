#include "clockobserver.h"

#include <iostream>

using namespace Stargazer::Audio;


ClockObserver::ClockObserver( Clock *source ) : m_clock(source) {
    
}

ClockObserver::~ClockObserver() {
    m_clock->removeObserver(this);
}

void ClockObserver::start() {
    std::cout << "ClockObserver::start: Attempted to start a clock observer. NBD."
    << std::endl;
}

void ClockObserver::stop() {
    std::cout << "ClockObserver::stop: Attempted to stop a clock observer. NBD."
    << std::endl;
}

void ClockObserver::reset(double) {
    std::cout << "ClockObserver::reset: Attempted to reset a clock observer. NBD."
    << std::endl;
}

void ClockObserver::advancePresentation(double) {
    std::cout << "ClockObserver::advancePresentation: Attempted to advance a "
    "clock observer. Umm?" << std::endl;
}

void ClockObserver::advancePipeline(double) {
    std::cout << "ClockObserver::advancePipeline: Attempted to advance a clock "
    "observer. Umm?" << std::endl;
}
