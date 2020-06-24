#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

#include "customer.h"

class SimulationSpy {
public:
    SimulationSpy(std::size_t L,
                  std::size_t maximum_system_customers,
                  const std::vector<std::string> & place_names,
                  std::uint32_t transient_period = 0)
    : L_(L)
    , transient_period_(transient_period)
    , place_names_(place_names)
    , system_customers_()
    , system_entered_customers_(0)
    , system_lost_customers_(0)
    , serviced_customers_(0)
    , total_service_time_(0)
    , total_system_time_(0)
    , additional_stats_()
    {
        // because the most customers in the server is known and small
        // (for project 1 it will be 101 I think). I'd rather just reserve the memory
        // upfront and not worry about rehashing
        system_customers_.reserve(maximum_system_customers);
        clear_stats();
    }

    std::uint32_t serviced_customers()
    {
        return serviced_customers_;
    }

    void on_customer_entering(const std::shared_ptr<Customer> & customer);
    void on_customer_exiting(const std::shared_ptr<Customer> & customer);

    std::unordered_map<std::string, float> customer_loss_rates() const;
    float average_service_time() const;
    std::unordered_map<std::string, float> average_waiting_times() const;
    float average_system_time() const;
    void print_proj1_stats() const;

private:
    void save_default_stats(const std::shared_ptr<Customer> & customer);
    void save_additional_stats(const std::shared_ptr<Customer> & customer);
    void on_transient_period_elapsed();

    void clear_stats()
    {
        system_entered_customers_ = 0;
        serviced_customers_ = 0;
        system_lost_customers_ = 0;
        total_service_time_ = 0;
        total_system_time_ = 0;
        for (const auto & name : place_names_) {
            waiting_times_by_queue_[name] = 0;
            total_entrances_by_queue_[name] = 0;
            unique_customers_by_queue_[name] = 0;
            losses_by_queue_[name] = 0;
        }
    }

    std::size_t L_;
    std::uint32_t transient_period_;
    std::vector<std::string> place_names_;

    std::unordered_map<std::uint32_t, std::shared_ptr<Customer>> system_customers_;

    // stats that must be cleared
    std::unordered_map<std::string, float> waiting_times_by_queue_;
    std::unordered_map<std::string, std::uint32_t> total_entrances_by_queue_;
    std::unordered_map<std::string, std::uint32_t> unique_customers_by_queue_;
    std::unordered_map<std::string, std::uint32_t> losses_by_queue_;
    std::uint32_t system_entered_customers_;
    std::uint32_t system_lost_customers_;
    std::uint32_t serviced_customers_;
    float total_service_time_;
    float total_system_time_;

    std::vector<std::string> additional_stats_;
};
