#include <iostream>

#include "simulation_spy.h"
#include "constants.h"

// TODO: unit test this class

void SimulationSpy::on_customer_entering(const std::shared_ptr<Customer> & customer)
{
    if (constants::DEBUG_ENABLED) {
        std::cout << "SimulationSpy::" << __func__
                  << " got " << customer->to_string()
                  << " old size:" << system_customers_.size() << std::endl;
    }
    ++system_entered_customers_;
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
    if (constants::DEBUG_ENABLED) {
        std::cout << "SimulationSpy::" << __func__
                  << " erasing stats!" << std::endl;
    }

    system_entered_customers_ = 0;
    serviced_customers_ = 0;
    system_lost_customers_ = 0;
    for (const auto & name : place_names_) {
        waiting_times_by_queue_[name] = 0;
        unique_customers_by_queue_[name] = 0;
        losses_by_queue_[name] = 0;
    }
    total_service_time_ = 0;
}

void SimulationSpy::save_default_stats(const std::shared_ptr<Customer> & customer)
{
    // TODO: consider making one big helper function that returns waiting times, entrances and losses
    // this will avoid iterating multiple times
    for (const auto & name : place_names_) {
        if (customer->entered(name)) {
            unique_customers_by_queue_.at(name) += 1;
        }
    }
    if (customer->serviced()) {
        ++serviced_customers_;

        for (const auto & name : place_names_) {
            waiting_times_by_queue_.at(name) += customer->waiting_time(name);
        }
        total_service_time_ += customer->service_time();
        total_system_time_ += customer->system_time();
    } else {
        losses_by_queue_.at(customer->dropped_by()) += 1;
        ++system_lost_customers_;
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

float SimulationSpy::average_service_time() const
{
    return total_service_time_ / serviced_customers_;
}

float SimulationSpy::total_waiting_time() const
{
    float total_waiting_time = 0;
    for (const auto & name : place_names_) {
        total_waiting_time += waiting_times_by_queue_.at(name);
    }
    return total_waiting_time;
}

float SimulationSpy::average_waiting_time() const
{
    return total_waiting_time() / serviced_customers_;
}

float SimulationSpy::average_system_time() const
{
    return total_system_time_ / serviced_customers_;
}

std::unordered_map<std::string, float> SimulationSpy::customer_loss_rates() const
{
    std::unordered_map<std::string, float> rates;
    for (const auto & name_and_unique_customers : unique_customers_by_queue_) {
        rates[name_and_unique_customers.first] = float(losses_by_queue_.at(name_and_unique_customers.first))
                                                 / name_and_unique_customers.second;
    }
    rates["overall"] = float(system_lost_customers_) / system_entered_customers_;
    return rates;
}

void SimulationSpy::print_proj1_stats() const
{
    std::cout << "CLR = "
              << system_lost_customers_
              << "/"
              << system_entered_customers_
              << " = "
              << float(system_lost_customers_) / system_entered_customers_
              << std::endl;

    std::cout << "Average Service Time = "
              << total_service_time_
              << "/"
              << serviced_customers_
              << " = "
              << average_service_time()
              << std::endl;

    std::cout << average_waiting_time()
              << total_waiting_time()
              << "/"
              << serviced_customers_
              << " = "
              << total_waiting_time() / serviced_customers_
              << std::endl;

    for (const auto & stat : additional_stats_) {
        std::cout << stat << std::endl;
    }
}
