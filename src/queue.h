#pragma once

#include <memory>
#include <functional>
#include <queue>

#include "customer.h"


class Queue {
public:
    Queue(std::size_t max_size,
          const CustomerRequest & exit_customer)
    : max_size_(max_size)
    , exit_customer_(exit_customer)
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

    void handle_requests();
    std::size_t max_size_;
    CustomerRequest exit_customer_;
    std::queue<std::function<void(std::shared_ptr<Customer>)>> requests_;
    std::queue<std::shared_ptr<Customer>> customers_;
};