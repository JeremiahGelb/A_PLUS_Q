#include "proj_2.h"

#include <iostream>

#include "simulation_timer.h"
#include "simulation_spy.h"
#include "prng.h"
#include "incoming_customers.h"
#include "queue.h"
#include "server.h"

namespace {
queueing::Discipline to_discipline(project2::Discipline discipline)
{
    switch(discipline) {
    case project2::Discipline::FCFS:
        return queueing::Discipline::FCFS;
    case project2::Discipline::LCFS_NP :
        return queueing::Discipline::LCFS_NP;
    case project2::Discipline::SJF_NP:
        return queueing::Discipline::SJF_NP;
    case project2::Discipline::PRIO_NP:
        return queueing::Discipline::PRIO_NP;
    case project2::Discipline::PRIO_P:
        return queueing::Discipline::PRIO_P;
    }
}

} // annonymous


namespace project2 {

void run_project_2(float lambda,
                   std::size_t max_cpu_queue_customers,
                   std::size_t max_io_queue_customers,
                   std::size_t customers_to_serve,
                   Mode mode,
                   Discipline discipline,
                   const int runs)
{
    std::vector<float> customer_loss_rates;
    std::vector<float> waiting_times; // TODO: (will need to split this up by priority)
    std::vector<float> system_times;
    for (auto i = 0; i < runs; ++i) {
        if (constants::PRINT_STATS) {
            std::cout << std::endl << "STARTING RUN: " << i << std::endl;
        }

        auto stat = do_one_run(lambda,
                               max_cpu_queue_customers,
                               max_io_queue_customers,
                               customers_to_serve,
                               mode,
                               discipline,
                               i*constants::SEED_OFFSET);

        customer_loss_rates.push_back(stat.customer_loss_rate());
        waiting_times.push_back(stat.average_waiting_time());
        system_times.push_back(stat.average_system_time());

        if (constants::PRINT_STATS) {
            std::cout << std::endl << "ENDING RUN: " << i << std::endl;
        }
    }

    if (constants::PRINT_STATS) {
        std::cout << "Customer Loss Rate: "
                  << statistics::confidence_interval_string(customer_loss_rates)
                  << std::endl;

        std::cout << "Waiting Time: "
                  << statistics::confidence_interval_string(waiting_times)
                  << std::endl;

        std::cout << "System Time "
                  << statistics::confidence_interval_string(system_times)
                  << std::endl;
    }
}

SimulationRunStats do_one_run(float lambda,
                              std::size_t max_cpu_queue_customers,
                              std::size_t /* max_io_queue_customers */,
                              std::size_t customers_to_serve,
                              Mode /* mode */,
                              Discipline discipline,
                              long seed_offset)
{
    // TODO make this do proj2
    // set up simulation
    constexpr float kMu = 1.0;
    long service_seed = 1234 + seed_offset;
    long arrival_seed = 4321 + seed_offset;

    SimulationTimer timer;

    constexpr std::size_t stats_index = 0; // used for project 1
    constexpr auto kTransientPeriod = 1000;
    auto spy = SimulationSpy(stats_index, max_cpu_queue_customers + 1, kTransientPeriod);

    auto exit_customer = [&spy] (const std::shared_ptr<Customer> & customer) {
        spy.on_customer_exiting(customer);
    };

    auto incoming_customers = IncomingCustomers(timer,
                                                ExponentialGenerator(lambda, arrival_seed));


    auto queue = Queue(max_cpu_queue_customers,
                       exit_customer,
                       ExponentialGenerator(kMu, service_seed),
                       to_discipline(discipline));

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

    while (spy.serviced_customers() < customers_to_serve) {
        timer.advance_time();
    }

    return SimulationRunStats(spy.customer_loss_rate(),
                              spy.average_waiting_time(),
                              spy.average_system_time());

}

} // project2