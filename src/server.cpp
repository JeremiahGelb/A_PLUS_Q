#include "server.h"
#include "debug.h"
#include <iostream>

void Server::start()
{
    customer_request_handler_([this](std::shared_ptr<Customer> customer){
        on_customer_entered(customer);
    });
}

void Server::on_customer_entered(std::shared_ptr<Customer> customer)
{
    auto departure_time = simulation_timer_.time() + customer->service_time();

    if(debug::DEBUG_ENABLED) {
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
                on_customer_entered(customer);
            });
        }
    ); 
}

void Server::on_customer_serviced(std::shared_ptr<Customer> customer) {
    if(debug::DEBUG_ENABLED) {
        std::cout << "TODODODO" << std::endl;
        std::cout << "serviced customer" << customer->to_string() << std::endl;
    }
}