#pragma once

#include <memory>
#include <functional>
#include <queue>

#include "customer.h"


class Queue {
public:
    Queue(std::size_t max_size)
    : max_size_(max_size)
    {}

    std::size_t size()
    {
        return customers_.size();
    }
    
    std::function<void(std::shared_ptr<Customer>)>
    accept_customer_callback(); // a getter for the function to put customers in the queue

    void 
    request_one_customer(const std::function<void(std::shared_ptr<Customer>)> & request);

private:
    void on_customer_rejected(const std::shared_ptr<Customer> & customer);
    void handle_requests();
    std::size_t max_size_;
    std::queue<std::function<void(std::shared_ptr<Customer>)>> requests_;
    std::queue<std::shared_ptr<Customer>> customers_;
};