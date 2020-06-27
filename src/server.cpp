#include "server.h"
#include "constants.h"
#include <iostream>

void Server::start()
{
    if (constants::DEBUG_ENABLED) {
        std::cout << "Server Started" << std::endl;
    }
    customer_request_handler_([this](std::shared_ptr<Customer> customer){
        on_customer_entered_server(customer);
    });
}

void Server::on_customer_entered_server(const std::shared_ptr<Customer> & customer)
{
    auto departure_time = simulation_timer_.time() + customer->service_time();

    if (constants::DEBUG_ENABLED) {
        std::cout << name_ << "::" << __func__
                  << " got customer: " << customer->to_string()
                  << " at time: " << simulation_timer_.time()
                  << " scheduling departure at time: " << departure_time
                  << std::endl;
    }

    // TODO put this back, and fix waiting time calculation with preempts
    /*
    customer->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::SERVER,
                                      name_,
                                      simulation_timer_.time()));
    */

    customer_ = customer;
    job_id_ = simulation_timer_.register_job(
        departure_time,
        [this] {
            on_customer_serviced(customer_); // service this customer

            customer_request_handler_([this](std::shared_ptr<Customer> customer){
                on_customer_entered_server(customer);
            });
        }
    );
}

std::shared_ptr<Customer> Server::attempt_preempt(const std::shared_ptr<Customer> & customer)
{
    if (customer->priority() >= customer_->priority()) {
        if (constants::DEBUG_ENABLED) {
            std::cout << "Server::" <<  __func__
                      << ": DID NOT replace {" << customer_->to_string()
                      << "} with {" << customer->to_string()
                      << "}" << std::endl;
        }

        return customer;
    } else {
        if (constants::DEBUG_ENABLED) {
            std::cout << "Server::" <<  __func__
                      << ": REPLACING {" << customer_->to_string()
                      << "} with {" << customer->to_string()
                      << "}" << std::endl;
        }

        auto old_departure_time = simulation_timer_.remove_job(job_id_);
        auto remaining_service_time = (old_departure_time - simulation_timer_.time());

        if (constants::DEBUG_ENABLED) {
            std::cout << "ID: " << customer_->id()
                    << " Old Departure Time: " << old_departure_time
                    << " Current Time: " << simulation_timer_.time()
                    << " Old service_time: " << customer_->service_time()
                    << " New Service Time" << remaining_service_time
                    << std::endl;
        }

        customer_->set_service_time(remaining_service_time);
        customer_->delete_last_event();
        // customer_->delete_last_event(); // delete the queue exit and server entrance
                                        // so that it's one wait in the server

        // TODO put this back, and fix waiting time calculation with preempts
        /*
        customer_->add_event(CustomerEvent(CustomerEventType::PREEMPTED_FROM,
                                           PlaceType::SERVER,
                                           name_,
                                           simulation_timer_.time()));
        */

        auto cached_customer = customer_;
        on_customer_entered_server(customer);

        if (constants::DEBUG_ENABLED) {
            std::cout << "Server::" << __func__
                      << " Returning: " << cached_customer->to_string()
                      << std::endl;
        }

        return cached_customer;
    }
}

void Server::on_customer_serviced(const std::shared_ptr<Customer> & customer)
{
    customer->set_serviced(true);
    customer->set_departure_time(simulation_timer_.time());

    // TODO put this back, and fix waiting time calculation with preempts
    /*
    if (constants::DEBUG_ENABLED) {
        std::cout << name_ << "::" << __func__
                  << " serviced customer: " << customer->to_string()
                  << std::endl;
    }
    */

    customer->add_event(CustomerEvent(CustomerEventType::EXITED,
                                      PlaceType::SERVER,
                                      name_,
                                      simulation_timer_.time()));

    exit_customer_(customer);
}
