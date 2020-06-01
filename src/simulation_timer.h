#pragma once

#include <vector>
#include <functional>
#include <map>

class SimulationTimer {
public:
    SimulationTimer();
    inline float time()
    {
        return time_;
    }

    inline void register_job(float start_time, std::function<void()> callback)
    {
        jobs_.insert({start_time, callback});
    }

    void advance_time();

private:
    float time_;
    std::multimap<uint32_t, std::function<void()>> jobs_;
};