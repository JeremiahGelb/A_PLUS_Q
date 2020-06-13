#pragma once

#include <memory>
#include <functional>
#include <queue>
#include <vector>

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
          queueing::Discipline discipline = queueing::Discipline::FCFS)
    : max_size_(max_size)
    , exit_customer_(exit_customer)
    , discipline_(discipline)
    {}

    std::size_t size()
    {
        return customers_.size();
    }
    
    void 
    accept_customer(const std::shared_ptr<Customer> & customer);

    void
    request_one_customer(const CustomerRequest & request);

private:
    void on_customer_rejected(const std::shared_ptr<Customer> & customer);
    std::vector<std::shared_ptr<Customer>>::iterator get_customer_iterator();

    void handle_requests();
    std::size_t max_size_;
    CustomerRequest exit_customer_;
    std::queue<std::function<void(std::shared_ptr<Customer>)>> requests_;
    std::vector<std::shared_ptr<Customer>> customers_;
    queueing::Discipline discipline_;
    /*
        to support different disciplines: managing customers in a vector. Because the queue size is unlikely to be very large
        vector will be faster that std::list even for random insert/remove

        https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md
        SL.con.2

        https://dzone.com/articles/c-benchmark-–-stdvector-vs
    */
};