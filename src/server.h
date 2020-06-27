#pragma once

#include <memory>
#include <functional>

#include "simulation_timer.h"
#include "customer.h"

class Server {
public:
    Server(const SimulationTimer & simulation_timer,
           const CustomerRequestHandler & customer_request_handler,
           const CustomerRequest & exit_customer,
           const std::string & name = "Server")
    : simulation_timer_(simulation_timer)
    , customer_request_handler_(customer_request_handler)
    , exit_customer_(exit_customer)
    , name_(name)
    {}

    void start();

    std::shared_ptr<Customer> attempt_preempt(const std::shared_ptr<Customer> & customer);

private:
    void on_customer_entered_server(const std::shared_ptr<Customer> & customer);
    void on_customer_serviced(const std::shared_ptr<Customer> & customer);

    std::uint32_t job_id_ = 0;
    std::shared_ptr<Customer> customer_;

    const SimulationTimer & simulation_timer_;
    CustomerRequestHandler customer_request_handler_;
    CustomerRequest exit_customer_;
    const std::string name_;
};
