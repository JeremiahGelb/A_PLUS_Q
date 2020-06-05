#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

#include "customer.h"

class SimulationSpy {
public:
    SimulationSpy(std::size_t L,
                  std::size_t maximum_system_customers)
    : L_(L)
    {
        // because the most customers in the server is known and small
        // (for project 1 it will be 101 I think). I'd rather just reserve the memory
        // upfront and not worry about rehashing
        system_customers_.reserve(maximum_system_customers);
    }

    std::uint32_t serviced_customers()
    {
        return serviced_customers_;
    }

    void on_customer_entering(const std::shared_ptr<Customer> & customer);
    void on_customer_exiting(const std::shared_ptr<Customer> & customer);

    void print_stats();



private:
    void save_default_stats(const std::shared_ptr<Customer> & customer);
    void save_additional_stats(const std::shared_ptr<Customer> & customer);

    std::size_t L_;
    std::unordered_map<std::uint32_t, std::shared_ptr<Customer>> system_customers_;

    std::uint32_t entered_customers_;
    std::uint32_t serviced_customers_;
    std::uint32_t lost_customers_;
    float total_waiting_time_;
    float total_service_time_;

    std::vector<std::string> additional_stats_;

};