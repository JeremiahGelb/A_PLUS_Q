#include <iostream>

#include "queue.h"
#include "debug.h"

void Queue::accept_customer(const std::shared_ptr<Customer> & customer)
{
    if (size() >= max_size_) {
        on_customer_rejected(customer);
    } else {
        if (debug::DEBUG_ENABLED) {
            std::cout << "Queue::" << __func__
                      << " adding customer: " << customer->to_string()
                      << std::endl;
        }
        customers_.push(customer);
    }

    handle_requests();
}

void Queue::on_customer_rejected(const std::shared_ptr<Customer> & customer) {
    if (debug::DEBUG_ENABLED) {
        std::cout << "Queue was full. Rejected:" << customer->to_string() << std::endl;
    }
    customer->set_departure_time(customer->arrival_time());
    exit_customer_(customer);
}

void
Queue::request_one_customer(const CustomerRequest & request)
{
    requests_.push(request);
    handle_requests();
}

void Queue::handle_requests()
{
    if (debug::DEBUG_ENABLED) {
        std::cout << "Queue::" << __func__ << " entered with: " 
                  << customers_.size() << " customers and " 
                  << requests_.size() << " requests" << std::endl;
    }

    while ((!customers_.empty()) && (!requests_.empty())) {
        auto customer = customers_.front();
        auto request = requests_.front();


        if (debug::DEBUG_ENABLED) {
            std::cout << "Queue::" << __func__ 
                      << " delivering_customer: " << customer->to_string() << std::endl;
        }

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

