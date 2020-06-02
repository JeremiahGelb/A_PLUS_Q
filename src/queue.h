#pragma once

#include <memory>
#include <functional>
#include <queue>

#include "customer.h"


class Queue {
public:
    Queue();
    
    std::function<void(std::shared_ptr<Customer>)>
    accept_customer_callback(); // a getter for the function to put customers in the queue

    void 
    request_one_customer(const std::function<void(std::shared_ptr<Customer>)> & request);

private:
    std::queue<std::function<void(std::shared_ptr<Customer>)>> requests;
    std::queue<std::shared_ptr<Customer>> customers;
};