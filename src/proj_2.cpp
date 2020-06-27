#include "proj_2.h"

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
    std::map<std::string, std::map<std::uint32_t, std::vector<float>>> customer_loss_rates;
    std::map<std::string, std::map<std::uint32_t, std::vector<float>>> average_waiting_times;
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
                              std::size_t max_cpu_queue_customers,
                              std::size_t max_io_queue_customers,
                              std::size_t customers_to_serve,
                              Mode mode,
                              Discipline discipline,
                              long seed_offset)
{
    switch (mode) {
    case Mode::MM1:
        return do_m_m_1_k(lambda,
                          max_cpu_queue_customers,
                          customers_to_serve,
                          discipline,
                          seed_offset);
    case Mode::CPU:
        return do_web_server(lambda,
                             max_cpu_queue_customers,
                             max_io_queue_customers,
                             customers_to_serve,
                             seed_offset);
    }
}

SimulationRunStats do_m_m_1_k(float lambda,
                              std::size_t max_cpu_queue_customers,
                              std::size_t customers_to_serve,
                              Discipline discipline,
                              long seed_offset)
{
    constexpr float kMu = 1.0;
    long service_seed = 1111 + seed_offset;
    long arrival_seed = 2222 + seed_offset;
    long priority_seed = 3333 + seed_offset;

    SimulationTimer timer;

    constexpr std::size_t stats_index = 0; // used for project 1
    constexpr auto kTransientPeriod = 1000;

    const std::string kQueueName = "Queue";
    auto spy = SimulationSpy(stats_index,
                             max_cpu_queue_customers + 1,
                             {kQueueName},
                             kTransientPeriod);

    auto exit_customer = [&spy] (const std::shared_ptr<Customer> & customer) {
        spy.on_customer_exiting(customer);
    };


    std::function<std::uint32_t()> generate_priority;
    constexpr auto kMinPriority = 1;
    constexpr auto kMaxPriority = 4;
    auto priority_generator = UniformPriorityGenerator(kMinPriority, kMaxPriority, priority_seed);

    std::uint32_t min_priority = 0;
    std::uint32_t max_priority = 0;
    switch(discipline) {
    case project2::Discipline::FCFS:
    case project2::Discipline::LCFS_NP :
    case project2::Discipline::SJF_NP:
        generate_priority = [] { return default_customer_priority(); };
        break;
    case project2::Discipline::PRIO_NP:
    case project2::Discipline::PRIO_P:
        generate_priority = [&priority_generator] { return priority_generator.generate(); };
        min_priority = 1;
        max_priority = 4;
        break;
    }

    IncomingCustomers incoming_customers(timer,
                                         ExponentialGenerator(lambda, arrival_seed),
                                         generate_priority);

    auto queue = Queue(max_cpu_queue_customers,
                       exit_customer,
                       ExponentialGenerator(kMu, service_seed),
                       [&timer]{ return timer.time(); },
                       to_discipline(discipline),
                       kQueueName,
                       min_priority,
                       max_priority);

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

    return SimulationRunStats(spy.customer_loss_rates(),
                              spy.average_waiting_times(),
                              spy.average_system_time());
}

SimulationRunStats do_web_server(float lambda,
                                 std::size_t max_cpu_queue_customers,
                                 std::size_t max_io_queue_customers,
                                 std::size_t customers_to_serve,
                                 long seed_offset)
{
    constexpr float kCpuMu = 1.0;
    constexpr float kIoMu = .5;
    long arrival_seed = 1111 + seed_offset;
    long cpu_service_seed = 2222 + seed_offset;
    long io_service_seed_1 = 3333 + seed_offset;
    long io_service_seed_2 = 4444 + seed_offset;
    long io_service_seed_3 = 5555 + seed_offset;
    long load_balancer_seed = 6666 + seed_offset;

    SimulationTimer timer;

    const std::string kCpuQueueName = "CPU_QUEUE";
    const std::string kIoQueueName1 = "IO_QUEUE1";
    const std::string kIoQueueName2 = "IO_QUEUE2";
    const std::string kIoQueueName3 = "IO_QUEUE3";

    constexpr std::size_t stats_index = 0; // used for project 1
    constexpr auto kTransientPeriod = 1000;
    auto spy = SimulationSpy(stats_index,
                             max_cpu_queue_customers
                             + 3*max_io_queue_customers
                             + 4,
                             {kCpuQueueName, kIoQueueName1, kIoQueueName2, kIoQueueName3},
                             kTransientPeriod);

    auto exit_customer = [&spy] (const std::shared_ptr<Customer> & customer) {
        spy.on_customer_exiting(customer);
    };

    auto incoming_customers = IncomingCustomers(timer,
                                                ExponentialGenerator(lambda, arrival_seed));


    auto cpu_queue = Queue(max_cpu_queue_customers,
                           exit_customer,
                           ExponentialGenerator(kCpuMu, cpu_service_seed),
                           [&timer]{ return timer.time(); },
                           queueing::Discipline::FCFS,
                           kCpuQueueName);
    auto io_queue_1 = Queue(max_io_queue_customers,
                            exit_customer,
                            ExponentialGenerator(kIoMu, io_service_seed_1),
                            [&timer]{ return timer.time(); },
                            queueing::Discipline::FCFS,
                            kIoQueueName1);
    auto io_queue_2 = Queue(max_io_queue_customers,
                            exit_customer,
                            ExponentialGenerator(kIoMu, io_service_seed_2),
                            [&timer]{ return timer.time(); },
                            queueing::Discipline::FCFS,
                            kIoQueueName2);
    auto io_queue_3 = Queue(max_io_queue_customers,
                            exit_customer,
                            ExponentialGenerator(kIoMu, io_service_seed_3),
                            [&timer]{ return timer.time(); },
                            queueing::Discipline::FCFS,
                            kIoQueueName3);

    CustomerRequest insert_into_cpu_queue = [&cpu_queue] (const std::shared_ptr<Customer> & customer) {
        cpu_queue.accept_customer(customer);
    };

    CustomerRequest insert_into_io_queue_1 = [&io_queue_1] (const std::shared_ptr<Customer> & customer) {
        io_queue_1.accept_customer(customer);
    };

    CustomerRequest insert_into_io_queue_2 = [&io_queue_2] (const std::shared_ptr<Customer> & customer) {
        io_queue_2.accept_customer(customer);
    };

    CustomerRequest insert_into_io_queue_3 = [&io_queue_3] (const std::shared_ptr<Customer> & customer) {
        io_queue_3.accept_customer(customer);
    };

    CustomerRequest insert_into_spy = [&spy] (const std::shared_ptr<Customer> & customer) {
        spy.on_customer_entering(customer);
    };

    // register queue to get customers
    incoming_customers.register_for_customers(insert_into_spy); // MUST REGISTER FIRST
    incoming_customers.register_for_customers(insert_into_cpu_queue);

    auto request_from_cpu_queue = [&cpu_queue] (const CustomerRequest & request) {
        cpu_queue.request_one_customer(request);
    };
    auto request_from_io_queue_1 = [&io_queue_1] (const CustomerRequest & request) {
        io_queue_1.request_one_customer(request);
    };
    auto request_from_io_queue_2 = [&io_queue_2] (const CustomerRequest & request) {
        io_queue_2.request_one_customer(request);
    };
    auto request_from_io_queue_3 = [&io_queue_3] (const CustomerRequest & request) {
        io_queue_3.request_one_customer(request);
    };

    constexpr float kIoQueue1Upper = .1;
    constexpr float kIoQueue2Upper = .2;
    constexpr float kIoQueue3Upper = .3;
    constexpr float kExitServicedUpper = 1.0;
    std::vector<RandomLoadBalancerTarget> targets = {std::make_pair(insert_into_io_queue_1, kIoQueue1Upper),
                                                     std::make_pair(insert_into_io_queue_2, kIoQueue2Upper),
                                                     std::make_pair(insert_into_io_queue_3, kIoQueue3Upper),
                                                     std::make_pair(exit_customer, kExitServicedUpper)};

    auto balancer = RandomLoadBalancer(targets, UniformGenerator(load_balancer_seed));
    CustomerRequest send_to_balancer = [&balancer] (const std::shared_ptr<Customer> & customer) {
        balancer.route_customer(customer);
    };

    auto cpu_server = Server(timer, request_from_cpu_queue, send_to_balancer, "CPU_SERVER");
    auto io_server_1 = Server(timer, request_from_io_queue_1, insert_into_cpu_queue, "IO_SERVER_1");
    auto io_server_2 = Server(timer, request_from_io_queue_2, insert_into_cpu_queue, "IO_SERVER_2");
    auto io_server_3 = Server(timer, request_from_io_queue_3, insert_into_cpu_queue, "IO_SERVER_3");

    // run simulation
    cpu_server.start();
    io_server_1.start();
    io_server_2.start();
    io_server_3.start();
    incoming_customers.start();

    while (spy.total_serviced_customers() < customers_to_serve) {
        timer.advance_time();
    }

    return SimulationRunStats(spy.customer_loss_rates(),
                              spy.average_waiting_times(),
                              spy.average_system_time());
}

} // project2
