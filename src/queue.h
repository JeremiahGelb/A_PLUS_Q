#pragma once

#include <memory>
#include <functional>
#include <queue>
#include <vector>
#include <iostream>

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

template <class ServiceTimeGenerator>
class Queue {
public:
    Queue(std::size_t max_size,
          const CustomerRequest & exit_customer,
          const ServiceTimeGenerator & service_time_generator,
          queueing::Discipline discipline = queueing::Discipline::FCFS)
    : max_size_(max_size)
    , exit_customer_(exit_customer)
    , service_time_generator_(service_time_generator)
    , discipline_(discipline)
    {}

    std::size_t size()
    {
        return customers_.size();
    }
    
    void accept_customer(const std::shared_ptr<Customer> & customer)
    {

        customer->set_service_time(service_time_generator_.generate());
        if (size() >= max_size_) {
            on_customer_rejected(customer);
        } else {
            if (constants::DEBUG_ENABLED) {
                std::cout << "Queue::" << __func__
                        << " adding customer: " << customer->to_string()
                        << std::endl;
            }
            switch(discipline_) {
            case queueing::Discipline::FCFS:
                customers_.push_back(customer);
                break;
            case queueing::Discipline::LCFS_NP:
                customers_.push_back(customer);
                break;
            case queueing::Discipline::SJF_NP:
            {
                auto pos = customers_.begin();
                for (; pos != customers_.end(); ++pos) {
                    if (customer->service_time() > (*pos)->service_time()) {
                        break;
                    }
                }
                customers_.insert(pos, customer);
                break;
            }
            case queueing::Discipline::PRIO_NP:
            case queueing::Discipline::PRIO_P:
                throw std::runtime_error("TODO: unimplimented");
            }
        }

        handle_requests();
    }

    void request_one_customer(const CustomerRequest & request)
    {
        requests_.push(request);
        handle_requests();
    }

private:
    void on_customer_rejected(const std::shared_ptr<Customer> & customer) {
        if (constants::DEBUG_ENABLED) {
            std::cout << "Queue was full. Rejected:" << customer->to_string() << std::endl;
        }
        customer->set_departure_time(customer->arrival_time());
        exit_customer_(customer);
    }

    std::vector<std::shared_ptr<Customer>>::iterator get_customer_iterator()
    {
        switch (discipline_) {
        case queueing::Discipline::FCFS:
            return customers_.begin();
        case queueing::Discipline::LCFS_NP:
            return customers_.end() - 1;
        case queueing::Discipline::SJF_NP:
            // sjf are inserted sorted
            return customers_.end() - 1;
        case queueing::Discipline::PRIO_NP:
        case queueing::Discipline::PRIO_P:
            throw std::runtime_error("TODO: unimplimented");
        }
    }

    void handle_requests()
    {
        if (constants::DEBUG_ENABLED) {
            std::cout << "Queue::" << __func__ << " entered with: " 
                    << customers_.size() << " customers and " 
                    << requests_.size() << " requests" << std::endl;
        }

        while ((!customers_.empty()) && (!requests_.empty())) {
            auto request = requests_.front();
            auto customer_iterator = get_customer_iterator();

            if (constants::DEBUG_ENABLED) {
                std::cout << "Queue::" << __func__ 
                        << " delivering_customer: " << (*customer_iterator)->to_string() << std::endl;
            }

            request(*customer_iterator);

            requests_.pop();
            customers_.erase(customer_iterator);
        }

        if (constants::DEBUG_ENABLED) {
            std::cout << __func__ << " exited with: " 
                    << customers_.size() << " customers and " 
                    << requests_.size() << " requests" << std::endl;
        }
    }
    std::size_t max_size_;
    CustomerRequest exit_customer_;
    std::queue<std::function<void(std::shared_ptr<Customer>)>> requests_;
    std::vector<std::shared_ptr<Customer>> customers_;
    ServiceTimeGenerator service_time_generator_;
    queueing::Discipline discipline_;
    /*
        to support different disciplines: managing customers in a vector. Because the queue size is unlikely to be very large
        vector will be faster that std::list even for random insert/remove

        https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md
        SL.con.2

        https://dzone.com/articles/c-benchmark-–-stdvector-vs
    */
};
