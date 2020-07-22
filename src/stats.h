#pragma once

#include <sstream>
#include <tgmath.h>
#include <unordered_map>

using queue_name_to_priority_to_stat = std::unordered_map<std::string, std::unordered_map<std::uint32_t, float>>;

class SimulationRunStats {
public:
    SimulationRunStats(const queue_name_to_priority_to_stat & customer_loss_rates,
                       const queue_name_to_priority_to_stat & average_waiting_times,
                       float average_system_time,
                       float average_service_time,
                       float simulation_end_time)
    : customer_loss_rates_(customer_loss_rates)
    , average_waiting_times_(average_waiting_times)
    , average_system_time_(average_system_time)
    , average_service_time_(average_service_time)
    , simulation_end_time_(simulation_end_time)
    {}

    queue_name_to_priority_to_stat customer_loss_rates()
    {
        return customer_loss_rates_;
    }

    queue_name_to_priority_to_stat average_waiting_times()
    {
        return average_waiting_times_;
    }

    float average_system_time()
    {
        return average_system_time_;
    }

    float average_service_time()
    {
        return average_service_time_;
    }

    float simulation_end_time()
    {
        return simulation_end_time_;
    }

    static std::uint32_t all_priorities() {
        return UINT32_MAX;
    }

    static std::string all_queues() {
        return "overall";
    }

private:
    queue_name_to_priority_to_stat customer_loss_rates_;
    queue_name_to_priority_to_stat average_waiting_times_;
    float average_system_time_;
    float average_service_time_;
    float simulation_end_time_;
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

    auto offset = z_for_95_percent_confidence * sqrt(variance) / sqrt(items.size());

    std::stringstream ss;
    ss << mean << " ± " << offset;
    return ss.str();
}

} // statistics
