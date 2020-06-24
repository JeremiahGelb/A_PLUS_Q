#pragma once

#include <sstream>
#include <tgmath.h>
#include <unordered_map>

class SimulationRunStats {
public:
    SimulationRunStats(const std::unordered_map<std::string, float> & customer_loss_rates,
                       const std::unordered_map<std::string, float> & average_waiting_times,
                       float average_system_time)
    : customer_loss_rates_(customer_loss_rates)
    , average_waiting_times_(average_waiting_times)
    , average_system_time_(average_system_time)
    {}

    std::unordered_map<std::string, float> customer_loss_rates()
    {
        return customer_loss_rates_;
    }

    std::unordered_map<std::string, float> average_waiting_times()
    {
        return average_waiting_times_;
    }

    float average_system_time()
    {
        return average_system_time_;
    }

private:
    std::unordered_map<std::string, float> customer_loss_rates_;
    std::unordered_map<std::string, float> average_waiting_times_;
    float average_system_time_;
};

namespace statistics {

// TODO: unit test all these
template <class Container, class T = float>
T sample_mean(const Container & items)
{
    T sum = 0;
    for (const auto & item : items) {
        sum += item;
    }

    return sum / items.size();
}

template <class Container, class T = float>
T sample_variance(const Container & items, const T & mean)
{
    T sum_of_squared_differences = 0;
    for (const auto & item : items) {
        const auto difference = item - mean;
        sum_of_squared_differences += difference * difference;
    }

    return sum_of_squared_differences / (items.size() - 1);
}

template <class Container>
std::string confidence_interval_string(const Container & items)
{
    auto mean = sample_mean(items);
    auto variance = sample_variance(items, mean);

    constexpr auto z_for_95_percent_confidence = 1.960;

    auto offset = variance / sqrt(items.size());

    std::stringstream ss;
    ss << mean << " ± " << offset;
    return ss.str();
}

} // statistics
