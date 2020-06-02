#include <iostream>

#include "queue.h"
#include "debug.h"

std::function<void(std::shared_ptr<Customer>)>
Queue::accept_customer_callback()
{
    return [this] (std::shared_ptr<Customer> customer) {
        if (size() >= max_size_) {
            on_customer_rejected(customer);
        } else {
            customers_.push(customer);
        }

        handle_requests();
    };
}

void Queue::on_customer_rejected(const std::shared_ptr<Customer> & customer) {
    if (debug::DEBUG_ENABLED) {
        std::cout << "Queue was full. Rejected:" << customer->to_string() << std::endl;
        std::cout << "TODO -> handle rejected customers" << std::endl;
    }
}

void
Queue::request_one_customer(const std::function<void(std::shared_ptr<Customer>)> & request)
{
    requests_.push(request);
    handle_requests();
}

void Queue::handle_requests()
{
    if (debug::DEBUG_ENABLED) {
        std::cout << __func__ << " entered with: " 
                  << customers_.size() << " customers and " 
                  << requests_.size() << " requests" << std::endl;
    }

    while ((!customers_.empty()) && (!requests_.empty())) {
        auto customer = customers_.front();
        auto request = requests_.front();
        request(customer);

        requests_.pop();
        customers_.pop();
    }

    if (debug::DEBUG_ENABLED) {
        std::cout << __func__ << " exited with: " 
                  << customers_.size() << " customers and " 
                  << requests_.size() << " requests" << std::endl;
    }
}

