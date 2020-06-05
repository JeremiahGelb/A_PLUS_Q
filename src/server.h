#pragma once

#include <memory>
#include <functional>

#include "simulation_timer.h"
#include "customer.h"

class Server {
public:
    Server(const SimulationTimer & simulation_timer,
           const CustomerRequestHandler & customer_request_handler)
    : simulation_timer_(simulation_timer)
    , customer_request_handler_(customer_request_handler)
    {}

    void start();

private:
    void on_customer_entered(std::shared_ptr<Customer>);
    void on_customer_serviced(std::shared_ptr<Customer>);

    const SimulationTimer & simulation_timer_;
    CustomerRequestHandler customer_request_handler_;
};