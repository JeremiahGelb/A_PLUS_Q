#pragma once

#include <vector>
#include <functional>
#include <map>
#include <iostream>

#include  "constants.h"

class SimulationTimer {
public:
    SimulationTimer();
    inline float time() const
    {
        return time_;
    }

    inline void register_job(float start_time, std::function<void()> callback) const
    {
        if (constants::DEBUG_ENABLED) {
            std::cout << "SimulationTimer::" << __func__ 
                      << " registered job with start time: " << start_time
                      << " at time: " << time_ << std::endl;
        }
        jobs_.insert({start_time, callback});
    }

    void advance_time();

private:
    float time_;
    mutable std::multimap<float, std::function<void()>> jobs_;
};