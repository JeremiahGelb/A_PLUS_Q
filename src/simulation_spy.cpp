#include <iostream>

#include "simulation_spy.h"
#include "constants.h"

void SimulationSpy::on_customer_entering(const std::shared_ptr<Customer> & customer)
{
    if (constants::DEBUG_ENABLED) {
        std::cout << "SimulationSpy::" << __func__
                  << " got " << customer->to_string()
                  << " old size:" << system_customers_.size() << std::endl;
    }
    ++system_entered_customers_[customer->priority()];
    system_customers_.insert({customer->id(), customer});

    if (constants::DEBUG_ENABLED) {
        std::cout << "SimulationSpy::" << __func__
                  << " exited with new size:" << system_customers_.size() << std::endl;
    }

}

void SimulationSpy::on_customer_exiting(const std::shared_ptr<Customer> & customer)
{
    if (constants::DEBUG_ENABLED) {
        std::cout << "SimulationSpy::" << __func__
                  << " erasing " << customer->to_string()
                  << " old size: " << system_customers_.size() << std::endl;
    }

    const auto id = customer->id();

    if (system_customers_.count(id) == 1) {
        system_customers_.erase(id); // customer not in system
    } else {
        throw std::runtime_error("system_customers asked to delete customer it doesn't have");
    }

    if (id == L_
        || id == L_+1
        || id == L_+10
        || id == L_+11) {
        save_additional_stats(customer);
    }

    save_default_stats(customer);

    if (id == transient_period_ - 1) {
        on_transient_period_elapsed();
    }

    if (constants::DEBUG_ENABLED) {
        std::cout << "SimulationSpy::" << __func__
                  << " exited with new size:" << system_customers_.size() << std::endl;
    }
}

void SimulationSpy::on_transient_period_elapsed()
{
    // TODO: consider testing this (manually tested once)
    if (constants::DEBUG_ENABLED) {
        std::cout << "SimulationSpy::" << __func__
                  << " erasing stats!" << std::endl;
    }

    clear_stats();
}

void SimulationSpy::save_default_stats(const std::shared_ptr<Customer> & customer)
{
    // TODO: consider making one big helper function (on customer)
    // that returns waiting times, entrances and losses
    // this will avoid iterating multiple times
    if (constants::DEBUG_ENABLED) {
        std::cout << "SimulationSpy::" << __func__ << " entered" << std::endl;
    }

    auto priority = customer->priority();

    if (customer->serviced()) {
        ++serviced_customers_[priority];

        for (const auto & name : queue_names_) {
            auto entrances = customer->entrances(name);
            total_entrances_by_queue_[name][priority] += entrances;
            if (entrances > 0) {
                unique_customers_by_queue_[name][priority] += 1;
            }
            waiting_times_by_queue_.at(name)[priority] += customer->waiting_time(name);
        }
        total_service_time_ += customer->service_time();
        total_system_time_ += customer->system_time();
        if (service_time_percentile_to_value_) {
            save_slowdown(customer->total_waiting_time(), customer->service_time());
        }
    } else {
        const auto & dropper = customer->dropped_by();
        losses_by_queue_[dropper][priority] += 1;
        unique_customers_by_queue_[dropper][priority] += 1;
        ++system_lost_customers_[customer->priority()];
    }
    if (constants::DEBUG_ENABLED) {
        std::cout << "SimulationSpy::" << __func__ << " exited" << std::endl;
    }
}

void SimulationSpy::save_additional_stats(const std::shared_ptr<Customer> & customer)
{
    std::stringstream ss;
    ss << "Customer ID: "
       << customer->id()
       << ", Arrival Time: "
       << customer->arrival_time()
       << ", Service Time: "
       << customer->service_time()
       << ", Departure Time: "
       << customer->departure_time()
       << ", Customers in system: "
       << system_customers_.size();

    additional_stats_.push_back(ss.str());
}

void SimulationSpy::save_slowdown(float waiting_time, float service_time)
{
    int percentile = 0;

    // consider using binary search or stl algorithm
    for(; percentile < 99; ++percentile) {
        if (service_time <= service_time_percentile_to_value_((percentile + 1) * .009999999999999)) {
            break;
        }
    }

    auto &[total_slowdown, customer_count] = total_slowdown_and_customer_count_by_service_time_percentile_.at(percentile);

    total_slowdown += waiting_time / service_time;
    ++customer_count;
}

std::array<float, 100> SimulationSpy::average_slowdown_percentiles()
{
    std::array<float, 100> average_slowdown_percentiles;
    std::transform(total_slowdown_and_customer_count_by_service_time_percentile_.begin(),
                   total_slowdown_and_customer_count_by_service_time_percentile_.end(),
                   average_slowdown_percentiles.begin(),
                   [] (const std::pair<float, std::uint32_t> & slowdown_and_count) {
                       return slowdown_and_count.first / slowdown_and_count.second;
                   });

    return average_slowdown_percentiles;
}

float SimulationSpy::average_service_time() const
{
    return total_service_time_ / total_serviced_customers();
}

float SimulationSpy::average_system_time() const
{
    return total_system_time_ / total_serviced_customers();
}

queue_name_to_priority_to_stat
SimulationSpy::customer_loss_rates()
{
    queue_name_to_priority_to_stat rates;

    // iterate across queues, and collect stats for each queue
    for (auto & name_and_unique_customers_map : unique_customers_by_queue_) {
        auto & name = name_and_unique_customers_map.first;
        float name_losses = 0;
        float name_unique_customers = 0;
        for (auto & priority_and_unique_entrances : name_and_unique_customers_map.second) {
            auto priority = priority_and_unique_entrances.first;

            float losses = losses_by_queue_[name][priority];
            float unique_customers = priority_and_unique_entrances.second;

            rates[name][priority] = losses / unique_customers;
            name_losses += losses;
            name_unique_customers += unique_customers;
        }

        rates[name][SimulationRunStats::all_priorities()] = name_losses / name_unique_customers;
    }

    // iterate across priorities, and collect stats about all queues
    float total_system_losses = 0;
    float total_system_entrances = 0;
    for (const auto & priority_and_entrances : system_entered_customers_) {
        auto priority = priority_and_entrances.first;
        float losses = system_lost_customers_[priority];
        auto entrances = priority_and_entrances.second;

        rates[SimulationRunStats::all_queues()][priority] = losses / entrances;

        total_system_entrances += entrances;
        total_system_losses += losses;
    }

    rates[SimulationRunStats::all_queues()][SimulationRunStats::all_priorities()] = total_system_losses / total_system_entrances;
    return rates;
}


queue_name_to_priority_to_stat
SimulationSpy::average_waiting_times()
{
    queue_name_to_priority_to_stat average_times;
    for (auto & name_and_total_entrances_map : total_entrances_by_queue_) {
        auto & name = name_and_total_entrances_map.first;
        float name_waiting_time = 0;
        float name_total_entrances = 0;
        for (auto & priority_and_total_entrances : name_and_total_entrances_map.second) {
            auto priority = priority_and_total_entrances.first;
            auto waiting_time = waiting_times_by_queue_[name][priority];
            auto total_entrances = priority_and_total_entrances.second;
            auto average_waiting_time = waiting_time / total_entrances;
            auto serviced_customers = serviced_customers_[priority];
            auto average_enterances = float(total_entrances) / serviced_customers;
            average_times[name][priority] = average_waiting_time;
            average_times[SimulationRunStats::all_queues()][priority] += average_waiting_time * average_enterances;

            name_waiting_time += waiting_time;
            name_total_entrances += total_entrances;
        }
        // this is the average waiting time any customer expects to wait given they enter a given queue
        auto name_average_waiting_time = name_waiting_time / name_total_entrances;
        average_times[name][SimulationRunStats::all_priorities()] = name_average_waiting_time;
        // this is the average total waiting in all queues time from customer perspective
        auto name_average_enterances = name_total_entrances / total_serviced_customers();

        // TODO: actually propogate this correctly
        // std::cout << "name " << name << " average_enterances " << name_average_enterances << std::endl;

        average_times[SimulationRunStats::all_queues()][SimulationRunStats::all_priorities()] += name_average_waiting_time * name_average_enterances;
    }
    return average_times;
}

void SimulationSpy::print_proj1_stats()
{
    std::cout << "CLR = "
              << system_lost_customers_.at(default_customer_priority())
              << "/"
              << system_entered_customers_.at(default_customer_priority())
              << " = "
              << float(system_lost_customers_.at(default_customer_priority()))
                 / system_entered_customers_.at(default_customer_priority())
              << std::endl;

    std::cout << "Average Service Time = "
              << total_service_time_
              << "/"
              << total_serviced_customers()
              << " = "
              << average_service_time()
              << std::endl;

    std::cout << "Average Waiting Time: "
              << average_waiting_times()[SimulationRunStats::all_queues()][SimulationRunStats::all_priorities()]
              << std::endl;

    for (const auto & stat : additional_stats_) {
        std::cout << stat << std::endl;
    }
}
