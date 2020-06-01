#include <iostream>

#include "simulation_timer.h"

SimulationTimer::SimulationTimer()
: time_(0)
{
    std::cout << "Simulation Timer Created" << std::endl;
}

void SimulationTimer::advance_time()
{
    if (jobs_.empty()) {
       throw(std::runtime_error("jobs empty in advance time"));
    }

    auto jobs_iterator = jobs_.rbegin();
    auto soonest_time = jobs_iterator->first;
    time_ = soonest_time;
    for (; jobs_iterator != jobs_.rend(); ++jobs_iterator) {
        if (jobs_iterator->first == soonest_time) {
            jobs_iterator->second();
        } else {
            break;
        }
   }
   jobs_.erase(soonest_time);
}