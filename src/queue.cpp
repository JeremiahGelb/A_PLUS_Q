#include <iostream>

#include "queue.h"
#include "constants.h"

void Queue::accept_customer(const std::shared_ptr<Customer> & customer)
{
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

void Queue::on_customer_rejected(const std::shared_ptr<Customer> & customer) {
    if (constants::DEBUG_ENABLED) {
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

std::vector<std::shared_ptr<Customer>>::iterator Queue::get_customer_iterator()
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
