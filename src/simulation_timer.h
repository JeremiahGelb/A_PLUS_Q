#pragma once

class SimulationTimer {
public:
    SimulationTimer();
    inline std::uint32_t time(){
        return time_;
    }

private:
    std::uint32_t time_;
};