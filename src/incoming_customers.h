#pragma once

#include "simulation_timer.h"
#include "prng.h"
#include "customer.h"

namespace {

    constexpr auto kArbitrarySeedOffset = 1223334444;

} // anonymous

template <class ArrivalTimeGenerator, class ServiceTimeGenerator>
class IncomingCustomers {
public:
    IncomingCustomers(SimulationTimer & simulation_timer, long seed = 0)
    : simulation_timer_(simulation_timer)
    , arrival_time_generator_(ArrivalTimeGenerator(seed))
    , service_time_generator_(ServiceTimeGenerator(seed + kArbitrarySeedOffset))
    {}

    void register_for_customers(std::function<void(std::shared_ptr<customer::Customer>)> & callback)
    {
        customer_destinations_.push_back(callback);
    }

    void start()
    {

    }

private:
    void generate_customer()
    {
       // auto now = simulation_timer_.time();
    }

    std::vector<std::function<void(std::shared_ptr<customer::Customer>)>> customer_destinations_;
    SimulationTimer & simulation_timer_;
    ArrivalTimeGenerator arrival_time_generator_;
    ServiceTimeGenerator service_time_generator_;
};