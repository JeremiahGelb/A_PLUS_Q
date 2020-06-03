#pragma once

#include <vector>
#include <functional>
#include <map>
#include <iostream>

#include  "debug.h"

class SimulationTimer {
public:
    SimulationTimer();
    inline float time() const
    {
        return time_;
    }

    inline void register_job(float start_time, std::function<void()> callback) const
    {
        if (debug::DEBUG_ENABLED) {
            std::cout << __func__ << " called with start time: " << start_time << std::endl;
        }
        jobs_.insert({start_time, callback});
    }

    void advance_time();

private:
    float time_;
    mutable std::multimap<uint32_t, std::function<void()>> jobs_;
};