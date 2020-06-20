#pragma once

#include <sstream>
#include <tgmath.h>

class SimulationRunStats {
public:
    SimulationRunStats(float customer_loss_rate,
                       float average_waiting_time,
                       float average_system_time)
    : customer_loss_rate_(customer_loss_rate)
    , average_waiting_time_(average_waiting_time)
    , average_system_time_(average_system_time)
    {}

    float customer_loss_rate()
    {
        return customer_loss_rate_;
    }

    float average_waiting_time()
    {
        return average_waiting_time_;
    }

    float average_system_time()
    {
        return average_system_time_;
    }

private:
    float customer_loss_rate_;
    float average_waiting_time_; // TODO: (will have to revamp for multi queue)
    float average_system_time_;
};

namespace statistics {

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

} // stat