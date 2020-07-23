#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <utility>
#include <array>

#include "customer.h"
#include "stats.h"

class SimulationSpy {
public:
    SimulationSpy(std::size_t L,
                  std::size_t maximum_system_customers,
                  const std::vector<std::string> & queue_names,
                  std::uint32_t transient_period = 0,
                  const std::function<double(double)> service_time_percentile_to_value = nullptr)
    : L_(L)
    , transient_period_(transient_period)
    , service_time_percentile_to_value_(service_time_percentile_to_value)
    , queue_names_(queue_names)
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

    std::uint32_t total_serviced_customers() const
    {
        std::uint32_t total = 0;
        for (const auto & priority_and_serviced_customers : serviced_customers_) {
            total += priority_and_serviced_customers.second;
        }
        return total;
    }

    void on_customer_entering(const std::shared_ptr<Customer> & customer);
    void on_customer_exiting(const std::shared_ptr<Customer> & customer);

    queue_name_to_priority_to_stat customer_loss_rates();
    queue_name_to_priority_to_stat average_waiting_times();

    float average_service_time() const;

    float average_system_time() const;
    std::array<float, 100> average_slowdown_percentiles();

    void print_proj1_stats();

private:
    void save_default_stats(const std::shared_ptr<Customer> & customer);
    void save_additional_stats(const std::shared_ptr<Customer> & customer);
    void save_slowdown(float service_time, float waiting_time);
    void on_transient_period_elapsed();

    void clear_stats()
    {
        system_entered_customers_ = {};
        system_lost_customers_ = {};
        serviced_customers_ = {};

        total_service_time_ = 0; // TODO: this isn't right for cpu example
        total_system_time_ = 0;

        for (const auto & name : queue_names_) {
            waiting_times_by_queue_[name] = {};
            total_entrances_by_queue_[name] = {};
            unique_customers_by_queue_[name] = {};
            losses_by_queue_[name] = {};
            total_slowdown_and_customer_count_by_service_time_percentile_ = {};
        }

    }

    std::size_t L_;
    std::uint32_t transient_period_;
    const std::function<double(double)> service_time_percentile_to_value_;
    std::vector<std::string> queue_names_;

    std::unordered_map<std::uint32_t, std::shared_ptr<Customer>> system_customers_;

    // stats that must be cleared
    std::array<std::pair<float, std::uint32_t>, 100> total_slowdown_and_customer_count_by_service_time_percentile_;
    std::unordered_map<std::string, std::unordered_map<std::uint32_t, float>> waiting_times_by_queue_;

    // This does not count if they are dropped by the queue
    std::unordered_map<std::string, std::unordered_map<std::uint32_t, std::uint32_t>> total_entrances_by_queue_;

    // this DOES count if they are dropped by
    std::unordered_map<std::string, std::unordered_map<std::uint32_t, std:: uint32_t>> unique_customers_by_queue_;

    std::unordered_map<std::string, std::unordered_map<std::uint32_t, std::uint32_t>> losses_by_queue_;
    std::unordered_map<std::uint32_t, std::uint32_t> system_entered_customers_;
    std::unordered_map<std::uint32_t, std::uint32_t> system_lost_customers_;
    std::unordered_map<std::uint32_t, std::uint32_t> serviced_customers_;
    float total_service_time_;
    float total_system_time_;

    std::vector<std::string> additional_stats_;
};
