#include "proj_1.h"

#include <iostream>

#include "simulation_timer.h"
#include "simulation_spy.h"
#include "prng.h"
#include "incoming_customers.h"
#include "queue.h"
#include "server.h"

void run_project_1(const float lambda,
                   const std::size_t max_queue_customers,
                   const std::size_t customers_to_serve,
                   const std::size_t L)
{
    if (constants::DEBUG_ENABLED) {
        std::cout << __func__
                  << " lambda: " << lambda
                  << ", max queue customers: " << max_queue_customers
                  << ", customers_to_serve: " << customers_to_serve
                  << ", L" << L << std::endl;
    }

    // set up simulation
    constexpr float kMu = 1.0;
    constexpr long kServiceSeed = 1234 + constants::SEED_OFFSET;
    constexpr long kArrivalSeed = 4321 + constants::SEED_OFFSET;

    SimulationTimer timer;

    const std::string kQueueName = "Queue";
    auto spy = SimulationSpy(L, max_queue_customers + 1, {kQueueName});

    auto exit_customer = [&spy] (const std::shared_ptr<Customer> & customer) {
        spy.on_customer_exiting(customer);
    };

    auto incoming_customers = IncomingCustomers(timer,
                                                ExponentialGenerator(lambda, kArrivalSeed));


    auto queue = Queue(max_queue_customers,
                       exit_customer,
                       [gen = ExponentialGenerator(kMu, kServiceSeed)] {
                            return gen.generate();
                       },
                       [&timer]{ return timer.time(); },
                       queueing::Discipline::FCFS,
                       kQueueName);

    CustomerRequest insert_into_queue = [&queue] (const std::shared_ptr<Customer> & customer) {
        queue.accept_customer(customer);
    };

    CustomerRequest insert_into_spy = [&spy] (const std::shared_ptr<Customer> & customer) {
        spy.on_customer_entering(customer);
    };

    // register queue to get customers
    incoming_customers.register_for_customers(insert_into_spy); // MUST REGISTER FIRST
    incoming_customers.register_for_customers(insert_into_queue);

    auto request_from_queue = [&queue] (const CustomerRequest & request) {
        queue.request_one_customer(request);
    };

    auto server = Server(timer, request_from_queue, exit_customer);

    // run simulation
    server.start();
    incoming_customers.start();

    while (spy.total_serviced_customers() < customers_to_serve) {
        timer.advance_time();
    }

    if (constants::PRINT_STATS) {
        std::cout << "Lambda: " << lambda << std::endl;
        std::cout << "K: " << max_queue_customers << std::endl;
        std::cout << "C: " << customers_to_serve << std::endl;
        std::cout << "Master Clock Value: " << timer.time() << std::endl;
        spy.print_proj1_stats();
    }
}
