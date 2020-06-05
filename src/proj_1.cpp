#include "proj_1.h"


#include "simulation_timer.h"
#include "prng.h"
#include "incoming_customers.h"
#include "queue.h"
#include "server.h"

void run_project_1(const float lambda,
                   const std::size_t max_queue_customers,
                   const std::size_t customers_to_serve,
                   const std::size_t /*L*/)
{
    // set up simulation
    constexpr float kMu = 1.0;
    constexpr long kServiceSeed = 1234;
    constexpr long kArrivalSeed = 4321;

    SimulationTimer timer;

    auto incoming_customers = IncomingCustomers(timer,
                                                ExponentialGenerator(lambda, kArrivalSeed),
                                                ExponentialGenerator(kMu, kServiceSeed));

    auto queue = Queue(max_queue_customers);
    CustomerRequest insert_into_queue = [&queue] (std::shared_ptr<Customer> customer) {
        queue.accept_customer(customer);
    };

    // register queue to get customers
    incoming_customers.register_for_customers(insert_into_queue);


    auto request_from_queue = [&queue] (const CustomerRequest & request) {
        queue.request_one_customer(request);
    };

    auto server = Server(timer, request_from_queue);

    // run simulation
    server.start();
    incoming_customers.start();

    // FIX THIS LOGIC -> this should wait for C customers from the exit customer handler TODOs
    std::uint32_t served_customers = 0;
    while (served_customers < customers_to_serve) {
        timer.advance_time();
        ++served_customers;
    }

}