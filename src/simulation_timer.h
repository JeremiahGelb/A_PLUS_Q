#pragma once

#include <vector>
#include <functional>
#include <map>

class SimulationTimer {
public:
    SimulationTimer();
    inline std::uint32_t time(){
        return time_;
    }

    inline void register_job(uint32_t start_time, std::function<void()> callback)
    {
        jobs_.insert({start_time, callback});
    }

    void advance_time();

private:
    std::uint32_t time_;
    std::multimap<uint32_t, std::function<void()>> jobs_;
};