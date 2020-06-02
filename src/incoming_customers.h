#pragma once

#include "simulation_timer.h"
#include "prng.h"
#include "customer.h"
#include "debug.h"

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

    void register_for_customers(const std::function<void(std::shared_ptr<customer::Customer>)> & callback)
    {
        customer_destinations_.push_back(callback);
    }

    void start()
    {
        generate_customer();
    }

private:
    void generate_customer()
    {
        float current_time = simulation_timer_.time();

        if (debug::DEBUG_ENABLED) {
            std::cout << __func__ <<  " called at time: " << current_time << std::endl;
        }
        auto arrival_time = last_arrival_time_ + arrival_time_generator_.generate();
        auto service_time = service_time_generator_.generate();
        auto customer = customer::make_customer(id_, arrival_time, service_time);

        last_arrival_time_ = arrival_time;
        ++id_;

        simulation_timer_.register_job(
            arrival_time,
            [this, customer] {
                for (const auto & callback : customer_destinations_) {
                    callback(customer);
                }

                generate_customer();
            }
        );
    }

    std::vector<std::function<void(std::shared_ptr<customer::Customer>)>> customer_destinations_;
    SimulationTimer & simulation_timer_;
    ArrivalTimeGenerator arrival_time_generator_;
    ServiceTimeGenerator service_time_generator_;
    std::uint32_t id_ = 0;
    float last_arrival_time_ = 0;
};