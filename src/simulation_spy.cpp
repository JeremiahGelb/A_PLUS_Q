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
    ++entered_customers_;
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

    entered_customers_ = 0;
    serviced_customers_ = 0;
    lost_customers_ = 0;
    total_waiting_time_ = 0;
    total_service_time_ = 0;
}

void SimulationSpy::save_default_stats(const std::shared_ptr<Customer> & customer)
{
    if (customer->serviced()) {
        ++serviced_customers_;

        // only save these stats for serviced customers
        total_waiting_time_ += customer->waiting_time();
        total_service_time_ += customer->service_time();
        total_system_time_ += customer->system_time();
    } else {
        ++lost_customers_;
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

float SimulationSpy::customer_loss_rate() const
{
    return float(lost_customers_) / entered_customers_;
}

float SimulationSpy::average_service_time() const
{
    return total_service_time_ / serviced_customers_;
}

float SimulationSpy::average_waiting_time() const
{
    return total_waiting_time_ / serviced_customers_;
}

float SimulationSpy::average_system_time() const
{
    return total_system_time_ / serviced_customers_;
}

void SimulationSpy::print_stats() const
{
    std::cout << "CLR = "
              << lost_customers_
              << "/"
              << entered_customers_
              << " = "
              << customer_loss_rate()
              << std::endl;

    std::cout << "Average Service Time = "
              << total_service_time_
              << "/"
              << serviced_customers_
              << " = "
              << average_service_time()
              << std::endl;

    std::cout << average_waiting_time()
              << total_waiting_time_
              << "/"
              << serviced_customers_
              << " = "
              << total_waiting_time_ / serviced_customers_
              << std::endl;

    for (const auto & stat : additional_stats_) {
        std::cout << stat << std::endl;
    }
}