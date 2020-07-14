#pragma once

#include <memory>
#include <functional>
#include <queue>
#include <vector>
#include <iostream>
#include <map>

#include "constants.h"
#include "customer.h"

namespace queueing {

    enum class Discipline {
       FCFS,
       LCFS_NP,
       SJF_NP,
       PRIO_NP,
       PRIO_P
    };

} // queueing

class Queue {
public:
    Queue(std::size_t max_size,
          const CustomerRequest & exit_customer,
          const std::function<float()> & service_time_generator,
          const std::function<float()> & current_time,
          queueing::Discipline discipline = queueing::Discipline::FCFS,
          const std::string & name = "Queue",
          std::uint32_t minimum_priority = default_customer_priority(),
          std::uint32_t maximum_priority = default_customer_priority())
    : max_vector_size_(max_size / (maximum_priority - minimum_priority + 1))
    , exit_customer_(exit_customer)
    , generate_service_time_(service_time_generator)
    , current_time_(current_time)
    , discipline_(discipline)
    , name_(name)
    {
        if (max_size % (maximum_priority - minimum_priority + 1)) {
            throw std::invalid_argument("Number of priorities must evenly divide max_size");
        }
        for (std::uint32_t i = minimum_priority; i <= maximum_priority; ++i) {
            customers_[i] = {};
        }
    }

    std::size_t size()
    {
        std::size_t size = 0;
        for (const auto & priority_and_customers : customers_) {
            size += priority_and_customers.second.size();
        }
        return size;
    }

    bool empty()
    {
        for (const auto & priority_and_customers : customers_) {
            if (!priority_and_customers.second.empty()) {
                return false;
            }
        }
        return true;
    }

    void incoming_preempted_customer(const std::shared_ptr<Customer> & customer)
    {
        if (!customer) {
            throw std::runtime_error("incoming_preempted_customer got null customer");
        }
        if (constants::DEBUG_ENABLED) {
            std::cout << "Queue::" << __func__
                      << ": got {" << customer->to_string()
                      << "}" << std::endl;
        }
        auto & priority_vector = customers_.at(customer->priority());
        priority_vector.insert(priority_vector.begin(), customer);

        if (priority_vector.size() > max_vector_size_) {
            if (constants::DEBUG_ENABLED) {
                std::cout << "Queue::" << __func__
                        << " vector too large after performing preempt!"
                        << " Kicking: " << priority_vector.back()->to_string()
                        << std::endl;
            }

            on_customer_rejected(priority_vector.back());
            priority_vector.pop_back();
        }
    }

    void accept_customer(const std::shared_ptr<Customer> & customer)
    {
        auto & priority_vector = customers_.at(customer->priority());

        if (priority_vector.size() >= max_vector_size_) {
            on_customer_rejected(customer);
        } else {
            if (constants::DEBUG_ENABLED) {
                std::cout << name_ << "::" << __func__
                        << " adding customer: " << customer->to_string()
                        << std::endl;
            }
            customer->set_service_time(generate_service_time_());

            customer->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                              PlaceType::QUEUE,
                                              name_,
                                              current_time_()));

            switch (discipline_) {
            case queueing::Discipline::FCFS:
                priority_vector.push_back(customer);
                break;
            case queueing::Discipline::LCFS_NP:
                priority_vector.push_back(customer);
                break;
            case queueing::Discipline::SJF_NP:
            {
                auto pos = priority_vector.begin();
                for (; pos != priority_vector.end(); ++pos) {
                    if (customer->service_time() > (*pos)->service_time()) {
                        break;
                    }
                }
                priority_vector.insert(pos, customer);
                break;
            }
            case queueing::Discipline::PRIO_NP:
                priority_vector.push_back(customer);
                break;
            case queueing::Discipline::PRIO_P:
                if (attempt_preempt_ == nullptr) {
                    throw std::runtime_error("PRRIO_P Q running without preempt function");
                }

                if (priority_vector.empty() && requests_.empty()) {
                    // Add entrance event assuming preempt goes through (fix this?)
                    customer->add_event(CustomerEvent(CustomerEventType::EXITED,
                                                      PlaceType::QUEUE,
                                                      name_,
                                                      current_time_()));

                    auto swapped_customer = attempt_preempt_(customer);
                    if (swapped_customer != customer) {
                        incoming_preempted_customer(swapped_customer);
                    } else {
                        customer->delete_last_event();
                        priority_vector.push_back(customer);
                    }
                } else {
                    priority_vector.push_back(customer);
                }
                break;
            }
        }

        handle_requests();
    }

    void request_one_customer(const CustomerRequest & request)
    {
        requests_.push(request);
        handle_requests();
    }

    void register_for_preempts(const CustomerSwapFunction & attempt_preempt) {
        attempt_preempt_ = attempt_preempt;
    }

private:
    void on_customer_rejected(const std::shared_ptr<Customer> & customer) {
        if (constants::DEBUG_ENABLED) {
            std::cout << "Queue was full. Rejected:" << customer->to_string() << std::endl;
        }
        customer->set_departure_time(current_time_());
        customer->add_event(CustomerEvent(CustomerEventType::DROPPED_BY,
                                          PlaceType::QUEUE,
                                          name_,
                                          current_time_()));
        customer->set_serviced(false);
        exit_customer_(customer);
    }


    std::vector<std::shared_ptr<Customer>> & lowest_non_empty_customer_vector()
    {
        for (auto & priority_and_customer_vector : customers_) {
            if (!priority_and_customer_vector.second.empty()) {
                return priority_and_customer_vector.second;
            }
        }
        throw(std::runtime_error("lowest non empty called when empty"));
    }

    std::shared_ptr<Customer> return_and_pop_customer()
    {
        switch (discipline_) {
        case queueing::Discipline::FCFS:
        {
            auto & customer_vector = customers_.at(default_customer_priority());
            auto customer = customer_vector.front();
            customer_vector.erase(customer_vector.begin()); // oof the moves
            return customer;
        }
        case queueing::Discipline::LCFS_NP:
        case queueing::Discipline::SJF_NP:
        {
            auto & customer_vector = customers_.at(default_customer_priority());
            auto customer = customer_vector.back();
            customer_vector.pop_back();
            return customer;
        }
        case queueing::Discipline::PRIO_NP:
        case queueing::Discipline::PRIO_P:
        {
            auto & customer_vector = lowest_non_empty_customer_vector();
            auto customer = customer_vector.front();
            customer_vector.erase(customer_vector.begin()); // oof the moves
            return customer;
        }
        }
    }

    void handle_requests()
    {
        if (constants::DEBUG_ENABLED) {
            std::cout << name_ << "::" << __func__ << " entered with: "
                      << size() << " customers and "
                      << requests_.size() << " requests" << std::endl;
        }

        while (!empty() && !requests_.empty()) {
            auto request = requests_.front();
            auto customer = return_and_pop_customer();

            if (constants::DEBUG_ENABLED) {
                std::cout << name_ << "::" << __func__
                          << " delivering_customer: " << customer->to_string() << std::endl;
            }

            customer->add_event(CustomerEvent(CustomerEventType::EXITED,
                                              PlaceType::QUEUE,
                                              name_,
                                              current_time_()));
            request(customer);

            requests_.pop();
        }

        if (constants::DEBUG_ENABLED) {
            std::cout << name_ << "::" << __func__ << " exited with: "
                    << size() << " customers and "
                    << requests_.size() << " requests" << std::endl;
        }
    }
    std::size_t max_vector_size_;
    CustomerRequest exit_customer_;
    std::queue<std::function<void(std::shared_ptr<Customer>)>> requests_;
    std::map<std::uint32_t , std::vector<std::shared_ptr<Customer>>> customers_;
    std::function<float()> generate_service_time_;
    const std::function<float()> current_time_;
    queueing::Discipline discipline_;
    const std::string name_;

   CustomerSwapFunction attempt_preempt_ = nullptr;
};
