#include "proj_3.h"

#include <iostream>
#include <map>

#include "simulation_timer.h"
#include "simulation_spy.h"
#include "prng.h"
#include "incoming_customers.h"
#include "queue.h"
#include "server.h"
#include "random_load_balancer.h"

namespace {

queueing::Discipline to_discipline(project3::Discipline discipline)
{
    switch (discipline) {
    case project3::Discipline::FCFS:
        return queueing::Discipline::FCFS;
    case project3::Discipline::SJF_NP:
        return queueing::Discipline::SJF_NP;
    }
}

} // annonymous


namespace project3 {

void run_project_3(float lambda,
                   std::size_t customers_to_serve,
                   Discipline discipline,
                   Mode mode,
                   const int runs)
{
    std::map<std::string, std::map<std::uint32_t, std::vector<float>>> customer_loss_rates;
    std::map<std::string, std::map<std::uint32_t, std::vector<float>>> average_waiting_times;
    std::vector<float> system_times;
    for (auto i = 0; i < runs; ++i) {
        if (constants::PRINT_STATS) {
            std::cout << std::endl << "STARTING RUN: " << i << std::endl;
        }

        auto stat = do_one_run(lambda,
                               customers_to_serve,
                               discipline,
                               mode,
                               i*constants::SEED_OFFSET);

        for (const auto & name_and_clr_map : stat.customer_loss_rates()) {
            auto & name = name_and_clr_map.first;
            for (const auto & priority_and_clr : name_and_clr_map.second) {
                auto priority = priority_and_clr.first;
                auto clr = priority_and_clr.second;
                customer_loss_rates[name][priority].push_back(clr);
            }
        }

        for (const auto & name_and_time_map : stat.average_waiting_times()) {
            auto & name = name_and_time_map.first;
            for (const auto & priority_and_time : name_and_time_map.second) {
                auto priority = priority_and_time.first;
                auto time = priority_and_time.second;
                average_waiting_times[name][priority].push_back(time);
            }
        }

        system_times.push_back(stat.average_system_time());

        if (constants::PRINT_STATS) {
            std::cout << std::endl << "ENDING RUN: " << i << std::endl;
        }
    }

    auto to_string = [] (std::uint32_t priority) {
        if (priority == SimulationRunStats::all_priorities()) {
            return std::string("AVERAGE");
        } else {
            return std::to_string(priority);
        }
    };

    if (constants::PRINT_STATS) {
        std::cout << std::endl << "Customer Loss Rates:" << std::endl;
        for (const auto & name_and_clr_vector_map : customer_loss_rates) {
            std::cout << name_and_clr_vector_map.first << ":" << std::endl;

            for (const auto & priority_and_clr : name_and_clr_vector_map.second) {
                std::cout << "    Priority_" << to_string(priority_and_clr.first) << ":"
                          << " CLR: " << statistics::confidence_interval_string(priority_and_clr.second)
                          << std::endl;
            }
        }
        std::cout << std::endl;

        std::cout << std::endl << "Waiting Times: " << std::endl;

        for (const auto & name_and_time_vector_map : average_waiting_times) {
            std::cout << name_and_time_vector_map.first << ":" << std::endl;

            for (const auto & priority_and_time : name_and_time_vector_map.second) {
                std::cout << "    Priority_" << to_string(priority_and_time.first) << ":"
                          << " Waiting Time: " << statistics::confidence_interval_string(priority_and_time .second)
                          << std::endl;
            }
        }
        std::cout << std::endl;

        std::cout << "System Time "
                  << statistics::confidence_interval_string(system_times)
                  << std::endl;
    }
}


SimulationRunStats do_one_run(float lambda,
                              std::size_t customers_to_serve,
                              Discipline discipline,
                              Mode mode,
                              long seed_offset)
{
    constexpr float kMu = 1.0/3000;
    long arrival_seed = 1111 + seed_offset;
    long service_seed = 2222 + seed_offset;
    long load_balancer_seed = 6666 + seed_offset;

    SimulationTimer timer;

    const std::string kQueueName = "QUEUE";

    constexpr std::size_t stats_index = 0; // used for project 1
    constexpr auto kTransientPeriod = 1000;
    constexpr auto kInitialReserve = 1000; // arbitrary
    auto spy = SimulationSpy(stats_index,
                             kInitialReserve,
                             {kQueueName},
                             kTransientPeriod);

    auto exit_customer = [&spy] (const std::shared_ptr<Customer> & customer) {
        spy.on_customer_exiting(customer);
    };

    auto incoming_customers = IncomingCustomers(timer,
                                                ExponentialGenerator(lambda, arrival_seed));




    std::function<float()> service_time_generator;
    switch(mode) {
    case Mode::MM3:
        service_time_generator = [gen = ExponentialGenerator(kMu, service_seed)] {
            return gen.generate();
        };
        break;
    case Mode::MG3:
    case Mode::MG1:
        throw std::runtime_error("TODO pareto");
        break;
    }

    Queue queue(SIZE_MAX,
                exit_customer,
                service_time_generator,
                [&timer]{ return timer.time(); },
                to_discipline(discipline),
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


    std::vector<Server> servers;

    switch(mode) {
    case Mode::MM3:
    case Mode::MG3:
        servers.emplace_back(Server(timer, request_from_queue, exit_customer, "server1"));
        servers.emplace_back(Server(timer, request_from_queue, exit_customer, "server2"));
        servers.emplace_back(Server(timer, request_from_queue, exit_customer, "server3"));
        break;
    case Mode::MG1:
        servers.emplace_back(Server(timer, request_from_queue, exit_customer, "server"));
        break;
    }

    // run simulation
    for (auto & server : servers) {
        server.start();
    }

    incoming_customers.start();

    while (spy.total_serviced_customers() < customers_to_serve) {
        timer.advance_time();
    }

    return SimulationRunStats(spy.customer_loss_rates(),
                              spy.average_waiting_times(),
                              spy.average_system_time());
}

} // project3
