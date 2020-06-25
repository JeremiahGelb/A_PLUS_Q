#pragma once

#include "simulation_timer.h"
#include "prng.h"
#include "customer.h"
#include "priority_generator.h"
#include "constants.h"

namespace {

    constexpr auto kArbitrarySeedOffset = 1223334444;

} // anonymous

template <class ArrivalTimeGenerator, class PriorityGenerator = ConstantPriorityGenerator>
class IncomingCustomers {
public:
    IncomingCustomers(const SimulationTimer & simulation_timer,
                      const ArrivalTimeGenerator & arrival_time_generator,
                      const PriorityGenerator & priority_generator = ConstantPriorityGenerator())
    : simulation_timer_(simulation_timer)
    , arrival_time_generator_(arrival_time_generator)
    , priority_generator_(priority_generator)
    {}

    void register_for_customers(const CustomerRequest & callback)
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
        auto arrival_time = last_arrival_time_ + arrival_time_generator_.generate();
        auto customer = make_customer(id_, arrival_time, priority_generator_.generate());

        last_arrival_time_ = arrival_time;
        ++id_;

        if (constants::DEBUG_ENABLED) {
            std::cout << "IncomingCustomers::" << __func__
                      << " called at time: " << simulation_timer_.time()
                      <<  " scheduling delivery of: " << customer->to_string()
                      << " for time: " << arrival_time << std::endl;
        }

        simulation_timer_.register_job(
            arrival_time,
            [this, customer] {

                if (constants::DEBUG_ENABLED) {
                    std::cout << "IncomingCustomers::" << __func__
                              <<  " delivering customer: " << customer->to_string()
                              << " at time: " << simulation_timer_.time() << std::endl;
                }

                for (const auto & callback : customer_destinations_) {
                    callback(customer);
                }

                generate_customer();
            }
        );
    }

    std::vector<CustomerRequest> customer_destinations_;
    const SimulationTimer & simulation_timer_;
    ArrivalTimeGenerator arrival_time_generator_;
    PriorityGenerator priority_generator_;
    std::uint32_t id_ = 0;
    float last_arrival_time_ = 0;
};
