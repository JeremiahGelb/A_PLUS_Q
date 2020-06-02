#include <iostream>

#include "simulation_timer.h"

SimulationTimer::SimulationTimer()
: time_(0)
{
    if (debug::DEBUG_ENABLED) {
            std::cout << "Simulation Timer Created" << std::endl;
    }
}

void SimulationTimer::advance_time()
{
    if (jobs_.empty()) {
       throw(std::runtime_error("jobs empty in advance time"));
    }

    if (debug::DEBUG_ENABLED) {
        std::cout << __func__ << " called at time:" << time_ << std::endl;
        std::cout << " with jobs:" << jobs_.size() << std::endl;
    }

    auto jobs_iterator = jobs_.begin();
    auto soonest_time = jobs_iterator->first;
    time_ = soonest_time;
    for (; jobs_iterator != jobs_.end(); ++jobs_iterator) {
        if (jobs_iterator->first == soonest_time) {
            jobs_iterator->second();
        } else {
            break;
        }
   }

    if (debug::DEBUG_ENABLED) {
        std::cout << __func__ << "advance time ended at time:" << time_ << std::endl;
        std::cout << " with jobs:" << jobs_.size() << std::endl;
    }

   jobs_.erase(soonest_time);
}