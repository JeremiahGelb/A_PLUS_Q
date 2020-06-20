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
        std::cout << __func__ 
                  << " got customer: " << customer->to_string() 
                  << "at time: " << simulation_timer_.time()
                  << " scheduling departure at time: " << departure_time
                  << std::endl;
    }

    simulation_timer_.register_job(
        departure_time,
        [this, customer] {
            on_customer_serviced(customer); // service this customer
            
            customer_request_handler_([this](std::shared_ptr<Customer> customer){
                on_customer_entered_server(customer);
            });
        }
    ); 
}

void Server::on_customer_serviced(const std::shared_ptr<Customer> & customer)
{
    customer->set_serviced(true);
    customer->set_departure_time(simulation_timer_.time());

    if (constants::DEBUG_ENABLED) {
        std::cout << "serviced customer: " << customer->to_string() << std::endl;
    }

    exit_customer_(customer);
}
