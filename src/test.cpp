#include <vector>
#include <functional>
#include <utility>
#include <string>
#include <iostream>
#include <unordered_map>

#include "test.h"
#include "simulation_timer.h"
#include "customer.h"
#include "prng.h"
#include "incoming_customers.h"
#include "queue.h"
#include "server.h"
#include "random_load_balancer.h"
#include "priority_generator.h"
#include "simulation_spy.h"

namespace {

auto make_exp_gen_lambda(float kLambda) {
    return [gen = ExponentialGenerator(kLambda)] { return gen.generate(); };
}

} //anonymous


namespace testing {

void run_all_tests()
{

    std::vector<std::pair<std::string, std::function<void()>>> tests = {
        std::make_pair("Blank", [] {}),
        std::make_pair("Test", test_tests),
        std::make_pair("Simulation Timer", test_simulation_timer),
        std::make_pair("Customer", test_customer),
        std::make_pair("prng", test_prng),
        std::make_pair("incoming customers", test_incoming_customers),
        std::make_pair("fcfs queue", test_fcfs_queue),
        std::make_pair("lcfs_queue", test_lcfs_queue),
        std::make_pair("sjf_queue", test_sjf_queue),
        std::make_pair("Server", test_server),
        std::make_pair("Random Load Balancer", test_random_load_balancer),
        std::make_pair("Customer events", test_customer_events),
        std::make_pair("Priority Generator", test_priority_generator),
        std::make_pair("Priority Non Preempt", test_prio_np_queue),
        std::make_pair("Priority Preempt", test_prio_p_queue),
        std::make_pair("Spy", test_spy),
        std::make_pair("Spy Odd", test_spy_odd_entrances),
        std::make_pair("Bounded Pareto", test_bounded_pareto)
    };

    auto failure_count = 0;
    auto unhandled_exception_count = 0;
    for (const auto & test : tests) {
        try {
            std::cout << std::endl << blue <<  "Starting test: " << def << test.first << std::endl;
            test.second();
            std::cout << green << "Passed test: " << def << test.first << std::endl;
        } catch (TestFailure & e) {
            ++failure_count;
            std::cout << red <<  "Failed test: " << def <<  test.first << std::endl;
            std::cout << "explanation: " << e.what() << std::endl;
        } catch (std::exception & e) {
            ++unhandled_exception_count;
            std::cout << red <<  "Caught unhandled exception in test: " << def <<  test.first << std::endl;
            std::cout << "explanation: " << e.what() << std::endl;
        }
    }

    constexpr auto kExpectedFailures = 0;
    constexpr auto kExpectedUnhandledExceptions = 0;

    if (failure_count == kExpectedFailures
        && unhandled_exception_count == kExpectedUnhandledExceptions) {
        std::cout << std::endl << green << "Overall Pass!" << def << std::endl;
    } else {
        std::cout << std::endl << red << "Overall Fail!" << def << std::endl;
    }
}

void test_tests()
{
    ASSERT(true);
    ASSERT(true, "With explanation");
    ASSERT_EQ(1, 1, "EQ");
    ASSERT_NEQ(2, 1, "NEQ");
    ASSERT_LT(1, 2, "LT");
    ASSERT_GT(2, 1, "GT");
}

void test_simulation_timer()
{
    SimulationTimer timer;
    ASSERT_EQ(timer.time(), float(0), "initial time wasn't 0");

    std::vector<std::uint32_t> call_order;

    timer.register_job(timer.time() + 1.1, [&call_order] { call_order.push_back(1); });
    timer.register_job(timer.time() + 3.0, [&call_order] { call_order.push_back(4); });
    timer.register_job(timer.time() + 2.2, [&call_order] { call_order.push_back(2); });
    timer.register_job(timer.time() + 2.2, [&call_order] { call_order.push_back(3); });

    auto remove_id = timer.register_job(timer.time() + 2.0,
                                        [] { ASSERT(false, "this job should get removed"); });

    ASSERT(call_order.empty(), "empty at start");

    timer.advance_time();
    ASSERT_EQ(call_order.size(), std::size_t(1), "first call did first job");

    timer.remove_job(remove_id);

    timer.advance_time();
    ASSERT_EQ(call_order.size(), std::size_t(3), "second call did two jobs");

    timer.advance_time();
    ASSERT_EQ(call_order.size(), std::size_t(4), "third call did last job");

    for (std::uint32_t i = 0; i < 4; ++i) {
        ASSERT_EQ(call_order[i], i+1, "Call order was correct");
    }

    try {
        timer.advance_time();
        timer.remove_job(1000);
        ASSERT(false, "expected to throw runtime error");
    } catch(std::runtime_error &) {}

}

void test_customer()
{
    constexpr std::uint32_t kFirstCustomerId = 1;
    constexpr float kFirstCustomerArrivalTime = 2.0;
    constexpr std::uint32_t kPriority = 999;

    constexpr auto kDefaultServiced = false;
    constexpr float kDefaultServiceTime = 0;
    constexpr float kDefaultDepartureTime = 0;

    auto first_customer = make_customer(kFirstCustomerId, kFirstCustomerArrivalTime, kPriority);

    ASSERT_EQ(first_customer->id(), kFirstCustomerId, "initial id matches");
    ASSERT_EQ(first_customer->arrival_time(), kFirstCustomerArrivalTime, "initial arrival time matches");
    ASSERT_EQ(first_customer->priority(), kPriority, "priority matches");
    ASSERT_EQ(first_customer->service_time(), kDefaultServiceTime, "initial service time matches");
    ASSERT_EQ(first_customer->serviced(), kDefaultServiced, "initial serviced matches");
    ASSERT_EQ(first_customer->departure_time(), kDefaultDepartureTime, "initial departure time matches");

    constexpr auto kNewServiced = true;
    constexpr float kNewDepartureTime = 10.5;
    constexpr float kNewServiceTime = 1.234;

    first_customer->set_serviced(kNewServiced);
    first_customer->set_departure_time(kNewDepartureTime);
    first_customer->set_service_time(kNewServiceTime);

    ASSERT_EQ(first_customer->serviced(), kNewServiced, "new serviced matches");
    ASSERT_EQ(first_customer->departure_time(), kNewDepartureTime, "new departure time matches");
    ASSERT_EQ(first_customer->service_time(), kNewServiceTime, "new service time matches");

    auto first_customer_string = first_customer->to_string();
    // std::cout << first_customer_string << std::endl;
    const std::string kExpectedString = "id: 1 Arrival Time: 2 Priority: 999 Service Time: 1.234 Serviced: 1 Departure Time: 10.5";
    ASSERT_EQ(first_customer_string, kExpectedString, "to string works as expected");
}

void test_prng()
{
    constexpr long kSeed = 12345;
    constexpr float kLambda = 1;
    constexpr long kArbitratyOffset = 1000;

    auto default_exp_gen = [gen = ExponentialGenerator(kLambda)] {
        return gen.generate();
    };
    ExponentialGenerator seeded_exp_gen(kLambda, kSeed);
    ExponentialGenerator same_seeded_exp_gen(kLambda, kSeed);

    ExponentialGenerator bigger_lambda_exp_gen(10*kLambda, kSeed + kArbitratyOffset);

    auto first_default_float = default_exp_gen();
    auto second_default_float = default_exp_gen();

    auto first_seeded_float = seeded_exp_gen.generate();
    auto second_seeded_float = same_seeded_exp_gen.generate();

    ASSERT_NEQ(first_default_float, second_default_float,
               "sequantial generations shouldn't be equal");

    ASSERT_EQ(first_seeded_float, second_seeded_float,
              "Same seed = same number");

    auto small_lambda_sum = 0;
    auto big_lambda_sum = 0;
    for (auto i = 0; i < 30 ; ++i) {
        small_lambda_sum += seeded_exp_gen.generate();
        big_lambda_sum += bigger_lambda_exp_gen.generate();
    }

    ASSERT_GT(small_lambda_sum, big_lambda_sum, "large lambda produces small values");

    auto uniform_generator = UniformGenerator(kSeed);
    constexpr auto kUniformChecks = 100;
    for (auto i = 0; i < kUniformChecks; ++i) {
        auto uniform_float = uniform_generator.generate();
        ASSERT_GT(uniform_float, float(0.0), "Greater than 0");
        ASSERT_LT(uniform_float, float(1.0), "Less than 1");
    }
}

void test_incoming_customers()
{
    auto customer_list = std::vector<std::shared_ptr<Customer>>();

    auto customer_callback = [&customer_list] (const std::shared_ptr<Customer> & customer) {
        customer_list.push_back(customer);
    };

    SimulationTimer timer;

    auto incoming_customers = IncomingCustomers(timer, ExponentialGenerator(1, 0));

    incoming_customers.register_for_customers(customer_callback);
    ASSERT_EQ(customer_list.size(), std::size_t(0), "empty before start");

    incoming_customers.start();
    ASSERT_EQ(customer_list.size(), std::size_t(0), "empty before run");

    timer.advance_time();
    ASSERT_EQ(customer_list.size(), std::size_t(1), "first customer added");

    timer.advance_time();
    ASSERT_EQ(customer_list.size(), std::size_t(2), "second customer added");

    // add some more customers
    timer.advance_time();
    timer.advance_time();
    timer.advance_time();
    timer.advance_time();
    timer.advance_time();

    float last_arrival_time = float(0.0);

    for (std::uint32_t i = 0; i < customer_list.size(); i++) {
        auto & customer = customer_list[i];
        ASSERT_EQ(customer->id(), i, "ids are sequential");

        auto arrival_time = customer->arrival_time();
        ASSERT_GT(arrival_time, last_arrival_time, "times are increasing");
        last_arrival_time = arrival_time;
    }

}

void test_fcfs_queue()
{
    constexpr std::size_t kMaxSize = 10;

    std::vector<std::shared_ptr<Customer>> rejected_customers;
    auto exit_customer = [&rejected_customers] (const std::shared_ptr<Customer> & customer) {
        rejected_customers.push_back(customer);
    };

    constexpr float kLambda = 1;
    auto queue = Queue(kMaxSize, exit_customer, make_exp_gen_lambda(kLambda), []{ return 0; });

    ASSERT_EQ(queue.size(), std::size_t(0), "queue empty at start");

    CustomerRequest insert = [&queue] (const std::shared_ptr<Customer> & customer) {
        queue.accept_customer(customer);
    };

    std::vector<std::shared_ptr<Customer>> received_customers;

    auto request = [&received_customers] (const std::shared_ptr<Customer> & customer) {
        received_customers.push_back(customer);
    };

    // testing basic functionality
    insert(make_customer(0, 1.1));
    ASSERT_EQ(queue.size(), std::size_t(1), "queue now has one customer");

    queue.request_one_customer(request);
    ASSERT_EQ(queue.size(), std::size_t(0), "queue empty again");
    ASSERT_EQ(received_customers.size(), std::size_t(1), "customer made it to vector");

    queue.request_one_customer(request);
    queue.request_one_customer(request);
    insert(make_customer(1, 1.1));
    ASSERT_EQ(queue.size(), std::size_t(0), "first pending request fulfilled");
    insert(make_customer(2, 1.1));
    ASSERT_EQ(queue.size(), std::size_t(0), "second pending request fulfilled");
    ASSERT_EQ(received_customers.size(), std::size_t(3), "got all those customers");


    // testing customers are delivered FIFO
    received_customers.clear();
    ASSERT(received_customers.size() == 0, "clear");
    insert(make_customer(0, 1.1));
    insert(make_customer(1, 1.1));
    insert(make_customer(2, 1.1));
    queue.request_one_customer(request);
    queue.request_one_customer(request);
    queue.request_one_customer(request);
    std::uint32_t counter = 0;
    for (const auto & customer : received_customers) {
        ASSERT_EQ(customer->id(), counter, "customers are fifo");
        ASSERT_NEQ(customer->service_time(), float(0.0), "queue sets service time");
        ++counter;
    }

    // testing requests are handled FIFO
    std::vector<int> call_order;
    auto request_1 = [&call_order] (const std::shared_ptr<Customer> &) {
        call_order.push_back(0);
    };
    auto request_2 = [&call_order] (const std::shared_ptr<Customer> &) {
        call_order.push_back(1);
    };
    queue.request_one_customer(request_1);
    queue.request_one_customer(request_2);
    insert(make_customer(0, 1.1));
    insert(make_customer(1, 1.1));
    ASSERT_EQ(call_order.size(), std::size_t(2), "both got handled");
    ASSERT_EQ(call_order[0], 0, "first handled first");
    ASSERT_EQ(call_order[1], 1, "second handled second");

    // testing max size
    ASSERT_EQ(queue.size(), std::size_t(0), "queue empty before testing max");
    for (std::size_t i = 0; i < kMaxSize; i++) {
        insert(make_customer(0, 1.1));
    }

    ASSERT_EQ(rejected_customers.size(), std::size_t(0), "no rejected customers");
    ASSERT_EQ(queue.size(), kMaxSize, "queue is at max");
    insert(make_customer(0, 1.1));
    ASSERT_EQ(queue.size(), kMaxSize, "queue didn't excede max");
    ASSERT_EQ(rejected_customers.size(), std::size_t(1), "one rejected customer");
}

void test_lcfs_queue()
{
    // Made the choice to just copy paste rather than making a base test
    constexpr std::size_t kMaxSize = 10;

    std::vector<std::shared_ptr<Customer>> rejected_customers;
    auto exit_customer = [&rejected_customers] (const std::shared_ptr<Customer> & customer) {
        rejected_customers.push_back(customer);
    };

    constexpr float kLambda = 1;
    auto queue = Queue(kMaxSize,
                       exit_customer,
                       make_exp_gen_lambda(kLambda),
                       []{ return 0; },
                       queueing::Discipline::LCFS_NP);

    ASSERT_EQ(queue.size(), std::size_t(0), "queue empty at start");

    CustomerRequest insert = [&queue] (const std::shared_ptr<Customer> & customer) {
        queue.accept_customer(customer);
    };

    std::vector<std::shared_ptr<Customer>> received_customers;

    auto request = [&received_customers] (const std::shared_ptr<Customer> & customer) {
        received_customers.push_back(customer);
    };

    // testing basic functionality
    insert(make_customer(0, 1.1));
    ASSERT_EQ(queue.size(), std::size_t(1), "queue now has one customer");

    queue.request_one_customer(request);
    ASSERT_EQ(queue.size(), std::size_t(0), "queue empty again");
    ASSERT_EQ(received_customers.size(), std::size_t(1), "customer made it to vector");

    queue.request_one_customer(request);
    queue.request_one_customer(request);
    insert(make_customer(1, 1.1));
    ASSERT_EQ(queue.size(), std::size_t(0), "first pending request fulfilled");
    insert(make_customer(2, 1.1));
    ASSERT_EQ(queue.size(), std::size_t(0), "second pending request fulfilled");
    ASSERT_EQ(received_customers.size(), std::size_t(3), "got all those customers");


    // testing customers are delivered FIFO
    received_customers.clear();
    ASSERT(received_customers.size() == 0, "clear");
    insert(make_customer(2, 1.1));
    insert(make_customer(1, 1.1));
    insert(make_customer(0, 1.1));
    queue.request_one_customer(request);
    queue.request_one_customer(request);
    queue.request_one_customer(request);
    std::uint32_t counter = 0;
    for (const auto & customer : received_customers) {
        ASSERT_EQ(customer->id(), counter, "customers are lifo");
        ASSERT_NEQ(customer->service_time(), float(0.0), "queue sets service time");
        ++counter;
    }

    // testing requests are handled FIFO
    std::vector<int> call_order;
    auto request_1 = [&call_order] (const std::shared_ptr<Customer> &) {
        call_order.push_back(0);
    };
    auto request_2 = [&call_order] (const std::shared_ptr<Customer> &) {
        call_order.push_back(1);
    };
    queue.request_one_customer(request_1);
    queue.request_one_customer(request_2);
    insert(make_customer(0, 1.1));
    insert(make_customer(1, 1.1));
    ASSERT_EQ(call_order.size(), std::size_t(2), "both got handled");
    ASSERT_EQ(call_order[0], 0, "first handled first");
    ASSERT_EQ(call_order[1], 1, "second handled second");

    // testing max size
    ASSERT_EQ(queue.size(), std::size_t(0), "queue empty before testing max");
    for (std::size_t i = 0; i < kMaxSize; i++) {
        insert(make_customer(0, 1.1));
    }

    ASSERT_EQ(rejected_customers.size(), std::size_t(0), "no rejected customers");
    ASSERT_EQ(queue.size(), kMaxSize, "queue is at max");
    insert(make_customer(0, 1.1));
    ASSERT_EQ(queue.size(), kMaxSize, "queue didn't excede max");
    ASSERT_EQ(rejected_customers.size(), std::size_t(1), "one rejected customer");
}

void test_sjf_queue()
{
    // Made the choice to just copy paste rather than making a base test
    constexpr std::size_t kMaxSize = 10;

    std::vector<std::shared_ptr<Customer>> rejected_customers;
    auto exit_customer = [&rejected_customers] (const std::shared_ptr<Customer> & customer) {
        rejected_customers.push_back(customer);
    };

    constexpr float kLambda = 1;
    auto queue = Queue(kMaxSize,
                       exit_customer,
                       make_exp_gen_lambda(kLambda),
                       []{ return 0; },
                       queueing::Discipline::SJF_NP);

    ASSERT_EQ(queue.size(), std::size_t(0), "queue empty at start");

    CustomerRequest insert = [&queue] (const std::shared_ptr<Customer> & customer) {
        queue.accept_customer(customer);
    };

    std::vector<std::shared_ptr<Customer>> received_customers;

    auto request = [&received_customers] (const std::shared_ptr<Customer> & customer) {
        received_customers.push_back(customer);
    };

    // testing basic functionality
    insert(make_customer(0, 1.1));
    ASSERT_EQ(queue.size(), std::size_t(1), "queue now has one customer");

    queue.request_one_customer(request);
    ASSERT_EQ(queue.size(), std::size_t(0), "queue empty again");
    ASSERT_EQ(received_customers.size(), std::size_t(1), "customer made it to vector");

    queue.request_one_customer(request);
    queue.request_one_customer(request);
    insert(make_customer(1, 1.1));
    ASSERT_EQ(queue.size(), std::size_t(0), "first pending request fulfilled");
    insert(make_customer(2, 1.1));
    ASSERT_EQ(queue.size(), std::size_t(0), "second pending request fulfilled");
    ASSERT_EQ(received_customers.size(), std::size_t(3), "got all those customers");


    // testing customers are delivered SJF
    received_customers.clear();
    ASSERT(received_customers.size() == 0, "clear");
    insert(make_customer(0, 1.1));
    insert(make_customer(1, 1.1));
    insert(make_customer(2, 1.1));
    queue.request_one_customer(request);
    queue.request_one_customer(request);
    queue.request_one_customer(request);
    std::uint32_t counter = 0;

    float previous_service_time = float(0.0);
    for (const auto & customer : received_customers) {
        auto incoming_service_time = customer->service_time();
        ASSERT_GT(incoming_service_time, previous_service_time, "customers are SJF");
        previous_service_time = incoming_service_time;
        ++counter;
    }

    // testing requests are handled FIFO
    std::vector<int> call_order;
    auto request_1 = [&call_order] (const std::shared_ptr<Customer> &) {
        call_order.push_back(0);
    };
    auto request_2 = [&call_order] (const std::shared_ptr<Customer> &) {
        call_order.push_back(1);
    };
    queue.request_one_customer(request_1);
    queue.request_one_customer(request_2);
    insert(make_customer(0, 1.1));
    insert(make_customer(1, 1.1));
    ASSERT_EQ(call_order.size(), std::size_t(2), "both got handled");
    ASSERT_EQ(call_order[0], 0, "first handled first");
    ASSERT_EQ(call_order[1], 1, "second handled second");

    // testing max size
    ASSERT_EQ(queue.size(), std::size_t(0), "queue empty before testing max");
    for (std::size_t i = 0; i < kMaxSize; i++) {
        insert(make_customer(0, 1.1));
    }

    ASSERT_EQ(rejected_customers.size(), std::size_t(0), "no rejected customers");
    ASSERT_EQ(queue.size(), kMaxSize, "queue is at max");
    insert(make_customer(0, 1.1));
    ASSERT_EQ(queue.size(), kMaxSize, "queue didn't excede max");
    ASSERT_EQ(rejected_customers.size(), std::size_t(1), "one rejected customer");
}

void test_prio_np_queue()
{
    std::vector<std::shared_ptr<Customer>> rejected_customers;
    auto exit_customer = [&rejected_customers] (const std::shared_ptr<Customer> & customer) {
        rejected_customers.push_back(customer);
    };

    constexpr std::size_t kBadMaxSize = 10;
    constexpr std::size_t kMaxSize = 12;
    constexpr float kLambda = 1;
    constexpr std::uint32_t kMin = 1;
    constexpr std::uint32_t kMax = 4;
    constexpr std::uint32_t kPriorities = 4;

    try {
        auto queue = Queue(kBadMaxSize,
                           exit_customer,
                           make_exp_gen_lambda(kLambda),
                           []{ return 0; },
                           queueing::Discipline::PRIO_NP,
                           "q",
                           kMin,
                           kMax);
        ASSERT(false, "bad size queue should throw");
    } catch (std::invalid_argument & e) {}

    auto queue = Queue(kMaxSize,
                       exit_customer,
                       make_exp_gen_lambda(kLambda),
                       []{ return 0; },
                       queueing::Discipline::PRIO_NP,
                       "q",
                       kMin,
                       kMax);

    ASSERT_EQ(queue.size(), std::size_t(0), "queue empty at start");

    CustomerRequest insert = [&queue] (const std::shared_ptr<Customer> & customer) {
        queue.accept_customer(customer);
    };

    std::vector<std::shared_ptr<Customer>> received_customers;

    auto request = [&received_customers] (const std::shared_ptr<Customer> & customer) {
        received_customers.push_back(customer);
    };

    // testing basic functionality
    insert(make_customer(0, 1.1, 1));
    ASSERT_EQ(queue.size(), std::size_t(1), "queue now has one customer");

    queue.request_one_customer(request);
    ASSERT_EQ(queue.size(), std::size_t(0), "queue empty again");
    ASSERT_EQ(received_customers.size(), std::size_t(1), "customer made it to vector");

    queue.request_one_customer(request);
    queue.request_one_customer(request);
    insert(make_customer(1, 1.1, 2));
    ASSERT_EQ(queue.size(), std::size_t(0), "first pending request fulfilled");
    insert(make_customer(2, 1.1, 2));
    ASSERT_EQ(queue.size(), std::size_t(0), "second pending request fulfilled");
    ASSERT_EQ(received_customers.size(), std::size_t(3), "got all those customers");


    // testing customers are delivered FIFO PRIO NP
    received_customers.clear();
    ASSERT(received_customers.size() == 0, "clear");
    insert(make_customer(3, 1.1 , 3));
    insert(make_customer(2, 1.1 , 2));
    insert(make_customer(0, 1.1, 1));
    insert(make_customer(1, 1.1, 1));
    queue.request_one_customer(request);
    queue.request_one_customer(request);
    queue.request_one_customer(request);
    queue.request_one_customer(request);
    std::uint32_t counter = 0;
    for (const auto & customer : received_customers) {
        ASSERT_EQ(customer->id(), counter, "customers are fifo within priority");
        ASSERT_NEQ(customer->service_time(), float(0.0), "queue sets service time");
        ++counter;
    }

    // testing requests are handled FIFO
    std::vector<int> call_order;
    auto request_1 = [&call_order] (const std::shared_ptr<Customer> &) {
        call_order.push_back(0);
    };
    auto request_2 = [&call_order] (const std::shared_ptr<Customer> &) {
        call_order.push_back(1);
    };
    queue.request_one_customer(request_1);
    queue.request_one_customer(request_2);
    insert(make_customer(0, 1.1, 1));
    insert(make_customer(1, 1.1, 1));
    ASSERT_EQ(call_order.size(), std::size_t(2), "both got handled");
    ASSERT_EQ(call_order[0], 0, "first handled first");
    ASSERT_EQ(call_order[1], 1, "second handled second");

    // testing max size
    ASSERT_EQ(queue.size(), std::size_t(0), "queue empty before testing max");
    for (std::size_t i = 0; i < kMaxSize/kPriorities; i++) {
        insert(make_customer(0, 1.1, 1));
        insert(make_customer(0, 1.1, 2));
        insert(make_customer(0, 1.1, 3));
        insert(make_customer(0, 1.1, 4));
    }

    ASSERT_EQ(rejected_customers.size(), std::size_t(0), "no rejected customers");
    ASSERT_EQ(queue.size(), kMaxSize, "queue is at max");
    insert(make_customer(0, 1.1, 1));
    insert(make_customer(0, 1.1, 2));
    insert(make_customer(0, 1.1, 3));
    insert(make_customer(0, 1.1, 4));
    ASSERT_EQ(queue.size(), kMaxSize, "queue didn't excede max");
    ASSERT_EQ(rejected_customers.size(), std::size_t(4), "four rejected customers");
}

void test_prio_p_queue()
{
    std::vector<std::shared_ptr<Customer>> rejected_customers;
    auto exit_customer = [&rejected_customers] (const std::shared_ptr<Customer> & customer) {
        rejected_customers.push_back(customer);
    };

    constexpr std::size_t kBadMaxSize = 10;
    constexpr std::size_t kMaxSize = 12;
    constexpr float kLambda = 1;
    constexpr std::uint32_t kMin = 1;
    constexpr std::uint32_t kMax = 4;
    constexpr std::uint32_t kPriorities = 4;

    try {
        auto queue = Queue(kBadMaxSize,
                           exit_customer,
                           make_exp_gen_lambda(kLambda),
                           []{ return 0; },
                           queueing::Discipline::PRIO_P,
                           "q",
                           kMin,
                           kMax);
        ASSERT(false, "bad size queue should throw");
    } catch (std::invalid_argument & e) {}

    auto queue = Queue(kMaxSize,
                       exit_customer,
                       make_exp_gen_lambda(kLambda),
                       []{ return 0; },
                       queueing::Discipline::PRIO_P,
                       "q",
                       kMin,
                       kMax);

    ASSERT_EQ(queue.size(), std::size_t(0), "queue empty at start");

    CustomerRequest insert = [&queue] (const std::shared_ptr<Customer> & customer) {
        queue.accept_customer(customer);
    };

    bool swap_enabled = false;
    std::shared_ptr<Customer> swap_target = make_customer(0,0,1);
    auto swap_call_count = 0;
    CustomerSwapFunction swap_if_enabled = [&swap_enabled,
                                            &swap_target,
                                            &swap_call_count] (const std::shared_ptr<Customer> & customer) {
        ++swap_call_count;
        if (swap_enabled) {
            auto cached_target = swap_target;
            swap_target = customer;
            cached_target->set_service_time(1);
            return cached_target;
        } else {
            return customer;
        }
    };

    queue.register_for_preempts(swap_if_enabled);

    std::vector<std::shared_ptr<Customer>> received_customers;

    auto request = [&received_customers] (const std::shared_ptr<Customer> & customer) {
        received_customers.push_back(customer);
    };

    // testing basic functionality
    insert(make_customer(0, 1.1, 1));
    ASSERT_EQ(queue.size(), std::size_t(1), "queue now has one customer");

    queue.request_one_customer(request);
    ASSERT_EQ(queue.size(), std::size_t(0), "queue empty again");
    ASSERT_EQ(received_customers.size(), std::size_t(1), "customer made it to vector");

    queue.request_one_customer(request);
    queue.request_one_customer(request);
    insert(make_customer(1, 1.1, 2));
    ASSERT_EQ(queue.size(), std::size_t(0), "first pending request fulfilled");
    insert(make_customer(2, 1.1, 2));
    ASSERT_EQ(queue.size(), std::size_t(0), "second pending request fulfilled");
    ASSERT_EQ(received_customers.size(), std::size_t(3), "got all those customers");


    // testing customers are delivered FIFO PRIO P
    constexpr std::uint32_t kRejectedId = 1234;

    received_customers.clear();
    ASSERT(received_customers.size() == 0, "clear");
    insert(make_customer(3, 1.1 , 3));
    insert(make_customer(1, 1.1 , 2));
    insert(make_customer(2, 1.1, 2));

    // third level 2 customer will get kicked when swap happens
    insert(make_customer(kRejectedId, 1.1, 2));
    ASSERT_EQ(rejected_customers.size(), std::size_t(0), "no rejected customers 1");

    swap_enabled = true;
    swap_target = make_customer(0, 1.1, 2);
    insert(make_customer(999, 1.1, 1));
    swap_enabled = false;

    ASSERT_EQ(rejected_customers.size(), size_t(1), "Customer booted when preempt comes in");
    ASSERT_EQ(rejected_customers[0]->id(), kRejectedId, "Customer booted when preempt comes in");
    rejected_customers.clear();

    queue.request_one_customer(request);
    queue.request_one_customer(request);
    queue.request_one_customer(request);
    queue.request_one_customer(request);
    std::uint32_t counter = 0;
    for (const auto & customer : received_customers) {
        ASSERT_EQ(customer->id(), counter, "customers are fifo within priority");
        ASSERT_NEQ(customer->service_time(), float(0.0), "queue sets service time");
        ++counter;
    }

    // testing requests are handled FIFO
    std::vector<int> call_order;
    auto request_1 = [&call_order] (const std::shared_ptr<Customer> &) {
        call_order.push_back(0);
    };
    auto request_2 = [&call_order] (const std::shared_ptr<Customer> &) {
        call_order.push_back(1);
    };
    queue.request_one_customer(request_1);
    queue.request_one_customer(request_2);
    insert(make_customer(0, 1.1, 1));
    insert(make_customer(1, 1.1, 1));
    ASSERT_EQ(call_order.size(), std::size_t(2), "both got handled");
    ASSERT_EQ(call_order[0], 0, "first handled first");
    ASSERT_EQ(call_order[1], 1, "second handled second");

    // testing max size
    ASSERT_EQ(queue.size(), std::size_t(0), "queue empty before testing max");
    for (std::size_t i = 0; i < kMaxSize/kPriorities; i++) {
        insert(make_customer(0, 1.1, 1));
        insert(make_customer(0, 1.1, 2));
        insert(make_customer(0, 1.1, 3));
        insert(make_customer(0, 1.1, 4));
    }

    ASSERT_EQ(rejected_customers.size(), std::size_t(0), "no rejected customers 2");
    ASSERT_EQ(queue.size(), kMaxSize, "queue is at max");
    insert(make_customer(0, 1.1, 1));
    insert(make_customer(0, 1.1, 2));
    insert(make_customer(0, 1.1, 3));
    insert(make_customer(0, 1.1, 4));
    ASSERT_EQ(queue.size(), kMaxSize, "queue didn't excede max");
    ASSERT_EQ(rejected_customers.size(), std::size_t(4), "four rejected customers");
}

void test_server()
{
    SimulationTimer timer;

    constexpr std::uint32_t kId = 0;
    constexpr auto kArrivalTime = 1.0;
    constexpr auto kServiceTime = 2.0;
    constexpr auto kPriority = 2.0;

    auto call_count = 0;
    CustomerRequestHandler customer_request_handler = [&call_count] (const CustomerRequest & request) {
        auto customer = make_customer(kId, kArrivalTime, kPriority);
        customer->set_service_time(kServiceTime);
        request(customer);
        ++call_count;
    };

    std::vector<std::shared_ptr<Customer>> serviced_customers;
    auto exit_customer = [&serviced_customers] (const std::shared_ptr<Customer> & customer) {
        serviced_customers.push_back(customer);
    };
    Server server(timer, customer_request_handler, exit_customer);

    ASSERT_EQ(call_count, 0, "uncalled");
    ASSERT_EQ(serviced_customers.size(), size_t(0), "none serviced");
    server.start();
    ASSERT_EQ(call_count, 1, "Called once");
    ASSERT_EQ(serviced_customers.size(), size_t(0), "none serviced (one in service)");

    timer.advance_time();
    ASSERT_EQ(call_count, 2, "Called twice");
    ASSERT_EQ(serviced_customers.size(), size_t(1), "one serviced)");

    auto higher_priority_customer = make_customer(kId + 1, kArrivalTime, kPriority - 1);
    auto lower_priority_customer = make_customer(kId + 2, kArrivalTime, kPriority + 1);
    auto swapped_customer = server.attempt_preempt(higher_priority_customer);

    ASSERT_EQ(swapped_customer->id(), kId, "default customer got swapped out");

    swapped_customer = server.attempt_preempt(lower_priority_customer);
    ASSERT_EQ(swapped_customer->id(), kId + 2, "Low prio customer got returned back");

    timer.advance_time();
    ASSERT_EQ(call_count, 3, "Called thrice");
    ASSERT_EQ(serviced_customers.size(), size_t(2), "two serviced)");

    ASSERT_EQ(serviced_customers.back()->id(), higher_priority_customer->id(), "next serviced customer is high prio");
}

void test_random_load_balancer()
{
    auto customer_list_1 = std::vector<std::shared_ptr<Customer>>();
    auto customer_list_2 = std::vector<std::shared_ptr<Customer>>();
    auto customer_list_3 = std::vector<std::shared_ptr<Customer>>();

    CustomerRequest to_list_1 = [&customer_list_1] (const std::shared_ptr<Customer> & customer) {
        customer_list_1.push_back(customer);
    };

    CustomerRequest to_list_2 = [&customer_list_2] (const std::shared_ptr<Customer> & customer) {
        customer_list_2.push_back(customer);
    };

    CustomerRequest to_list_3 = [&customer_list_3] (const std::shared_ptr<Customer> & customer) {
        customer_list_3.push_back(customer);
    };

    constexpr float kList1UpperProbability = .1;
    constexpr float kList2UpperProbability = .4;
    constexpr float kList3UpperProbability = 1.0;
    std::vector<RandomLoadBalancerTarget> targets = {std::make_pair(to_list_1, kList1UpperProbability),
                                                     std::make_pair(to_list_2, kList2UpperProbability),
                                                     std::make_pair(to_list_3, kList3UpperProbability)};

    auto balancer = RandomLoadBalancer(targets, UniformGenerator());

    constexpr auto kCustomers = 100;
    for (auto i = 0; i < kCustomers; ++i) {
        balancer.route_customer(make_customer(0,0));
    }

    if (constants::DEBUG_ENABLED) {
        std::cout << customer_list_1.size()
                  << " " << customer_list_2.size()
                  << " " << customer_list_3.size()
                  << std::endl;
    }

    ASSERT_GT(customer_list_2.size(), customer_list_1.size(), "~30% bigger than ~10%");
    ASSERT_GT(customer_list_3.size(), customer_list_2.size(), "~60% bigger than ~30%");
}

void test_customer_events()
{
    auto customer = make_customer(0,0);
    customer->set_serviced(true);

    constexpr float kEnteredQueue = 1.0;
    constexpr float kExitedQueue = 2.0;

    constexpr float kEnteredServer = 2.0;
    constexpr float kExitedServer = 5.0;

    std::string queue_name_1("queue1");
    std::string queue_name_2("queue2");
    std::string server_name("server");

    customer->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kEnteredQueue));
    customer->add_event(CustomerEvent(CustomerEventType::EXITED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kExitedQueue));
    customer->add_event(CustomerEvent(CustomerEventType::ENTERED, PlaceType::QUEUE, queue_name_2, kEnteredQueue));
    customer->add_event(CustomerEvent(CustomerEventType::EXITED, PlaceType::QUEUE, queue_name_2, kExitedQueue));

    customer->add_event(CustomerEvent(CustomerEventType::ENTERED, PlaceType::SERVER, server_name, kEnteredServer));
    customer->add_event(CustomerEvent(CustomerEventType::EXITED, PlaceType::SERVER, server_name, kExitedServer));

    ASSERT_EQ(customer->total_waiting_time(), 2*(kExitedQueue - kEnteredQueue), "total is 2");

    ASSERT_EQ(customer->waiting_time(queue_name_1), (kExitedQueue - kEnteredQueue), "queue1 is 1");

    ASSERT_EQ(customer->waiting_time(server_name), (kExitedServer - kEnteredServer), "server is 3");

    auto customer_string = customer->to_string(true);

    // std::cout << customer_string << std::endl;
    const std::string kExpectedString = "id: 0 Arrival Time: 0 Priority: 0 Service Time: 0 Serviced: 1 Departure Time: 0"
                                        "\nENTERED QUEUE queue1 at 1"
                                        "\nEXITED QUEUE queue1 at 2"
                                        "\nENTERED QUEUE queue2 at 1"
                                        "\nEXITED QUEUE queue2 at 2"
                                        "\nENTERED SERVER server at 2"
                                        "\nEXITED SERVER server at 5";
    ASSERT_EQ(customer_string, kExpectedString, "to string works as expected");

    // testing entered and entrances
    customer->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kEnteredQueue));
    customer->add_event(CustomerEvent(CustomerEventType::EXITED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kExitedQueue));

    ASSERT_EQ(customer->entered(queue_name_1), true, "It has entered queue1");
    ASSERT_EQ(customer->entered("Garbage"), false, "It has not entered garbage");
    ASSERT_EQ(customer->entrances(queue_name_1), std::uint32_t(2), "it entered queue 1 twice");

    customer->add_event(CustomerEvent(CustomerEventType::DROPPED_BY,
                                      PlaceType::QUEUE,
                                      queue_name_2,
                                      kExitedQueue));
    ASSERT_EQ(customer->dropped_by(), queue_name_2, "was dropped by queue 2");
}

void test_priority_generator()
{
    constexpr auto kSeed = 0;
    constexpr std::uint32_t kMin = 1;
    constexpr std::uint32_t kMax = 4;
    constexpr std::size_t kPriorities = (kMax - kMin + 1);
    auto uniform_priority_generator = UniformPriorityGenerator(kMin,
                                                               kMax,
                                                               kSeed);

    std::unordered_map<std::uint32_t, int> generated_priorities;
    constexpr std::uint32_t kRuns = 10000;
    for (std::uint32_t i = 0; i < kRuns; ++i) {
        generated_priorities[uniform_priority_generator.generate()]++;
    }

    ASSERT_EQ(generated_priorities.size(), kPriorities, "generated every priority");

    auto abs = [] (auto i) {
        return i < 0 ? -i : i;
    };

    for (std::uint32_t i = kMin; i <= kMax; ++i) {
        auto count = generated_priorities.at(i);
        constexpr int kExpectedCount = kRuns / kPriorities;
        constexpr int kMaxDifference = kExpectedCount / 50; // 2% error
        ASSERT_LT(abs(count - kExpectedCount), kMaxDifference, "count was close to expected");
    }
}

void test_spy()
{
    auto customer = make_customer(1, 0, 1);
    auto customer2 = make_customer(2, 0, 2);
    auto customer3 = make_customer(3, 0, 1);
    auto customer4 = make_customer(3, 0, 1);
    customer->set_serviced(true);
    customer2->set_serviced(true);
    customer3->set_serviced(false);
    customer4->set_serviced(true);

    constexpr float kEnteredQueue = 1.0;
    constexpr float kExitedQueue = 2.0;
    constexpr float kExitedQueueLong = 3.0;

    std::string queue_name_1("queue1");
    std::string queue_name_2("queue2");

    // Customer 1 enters queue1 twice, taking 1 second each
    // Customer 1 enters queue2 once, taking 1 second
    // Customer 1 has 3s of total waiting time
    customer->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kEnteredQueue));
    customer->add_event(CustomerEvent(CustomerEventType::EXITED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kExitedQueue));

    customer->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kEnteredQueue));
    customer->add_event(CustomerEvent(CustomerEventType::EXITED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kExitedQueue));

    customer->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::QUEUE,
                                      queue_name_2,
                                      kEnteredQueue));

    customer->add_event(CustomerEvent(CustomerEventType::EXITED,
                                      PlaceType::QUEUE,
                                      queue_name_2,
                                      kExitedQueue));

    // Customer 2 goes into queue 1 once, and it takes 2 seconds
    // Customer 2 goes into queue 2 once, and it takes 2 seconds
    // customer 2 is in system for 4 seconds total
    customer2->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kEnteredQueue));
    customer2->add_event(CustomerEvent(CustomerEventType::EXITED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kExitedQueueLong));

    customer2->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::QUEUE,
                                      queue_name_2,
                                      kEnteredQueue));
    customer2->add_event(CustomerEvent(CustomerEventType::EXITED,
                                      PlaceType::QUEUE,
                                      queue_name_2,
                                      kExitedQueueLong));

    // customer 3 gets dropped the second time through q1
    customer3->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kEnteredQueue));
    customer3->add_event(CustomerEvent(CustomerEventType::EXITED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kExitedQueue));
    customer3->add_event(CustomerEvent(CustomerEventType::DROPPED_BY,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kEnteredQueue));

    // Customer 4 enters queue1 twice, taking 1 second each
    // Customer 4 enters queue2 once, taking 1 second
    // Customer 4 has 3s of total waiting time
    customer4->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kEnteredQueue));
    customer4->add_event(CustomerEvent(CustomerEventType::EXITED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kExitedQueue));

    customer4->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kEnteredQueue));
    customer4->add_event(CustomerEvent(CustomerEventType::EXITED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kExitedQueue));

    customer4->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::QUEUE,
                                      queue_name_2,
                                      kEnteredQueue));

    customer4->add_event(CustomerEvent(CustomerEventType::EXITED,
                                      PlaceType::QUEUE,
                                      queue_name_2,
                                      kExitedQueue));

    constexpr auto kNoLVal = 0;
    constexpr auto kMaxSize = 100;
    constexpr auto kTransientPeriod = 1000;
    SimulationSpy spy(kNoLVal,
                      kMaxSize,
                      {queue_name_1, queue_name_2},
                      kTransientPeriod);

    spy.on_customer_entering(customer);
    spy.on_customer_exiting(customer);
    spy.on_customer_entering(customer2);
    spy.on_customer_exiting(customer2);
    spy.on_customer_entering(customer3);
    spy.on_customer_exiting(customer3);
    spy.on_customer_entering(customer4);
    spy.on_customer_exiting(customer4);

    queue_name_to_priority_to_stat waiting_times = spy.average_waiting_times();
    queue_name_to_priority_to_stat clrs = spy.customer_loss_rates();

    if (constants::DEBUG_ENABLED) {
        for (const auto & name_and_waiting_time_map : waiting_times) {
            std::cout << name_and_waiting_time_map.first << std::endl;
            for (const auto & priority_and_waiting_time : name_and_waiting_time_map.second) {
                std::cout << "    Priority-" << priority_and_waiting_time.first
                        << ": waiting_time = " << priority_and_waiting_time.second
                        << std::endl;
            }
        }

        for (const auto & name_and_waiting_clr_map : clrs) {
            std::cout << name_and_waiting_clr_map.first << std::endl;
            for (const auto & priority_and_clr : name_and_waiting_clr_map.second) {
                std::cout << "    Priority-" << priority_and_clr.first
                        << ": clr = " << priority_and_clr.second
                        << std::endl;
            }
        }
    }

    // RECAP
    // P1Customer 1 enters queue1 twice, taking 1 second each
    // P1Customer 1 enters queue2 once, taking 1 second
    // P1Customer 1 has 3s of total waiting time
    // P2Customer 2 goes into queue 1 once, and it takes 2 seconds
    // P2Customer 2 goes into queue 2 once, and it takes 2 seconds
    // P2customer 2 is in system for 4 seconds total
    // P1customer 3 is went into q1 once and was dropped the second time
    // P1Customer 4 enters queue1 twice, taking 1 second each
    // P1Customer 4 enters queue2 once, taking 1 second
    // P1Customer 4 has 3s of total waiting time

    ASSERT_EQ(waiting_times[queue_name_1][SimulationRunStats::all_priorities()], float(6.0/5), "queue one avg correct wt");
    ASSERT_EQ(waiting_times[queue_name_1][1], float(1), "queue one p1 correct wt");
    ASSERT_EQ(waiting_times[queue_name_1][2], float(2), "queue one p2 correct wt");

    ASSERT_EQ(waiting_times[queue_name_2][SimulationRunStats::all_priorities()], float(4.0/3), "queue two avg correct wt");
    ASSERT_EQ(waiting_times[queue_name_2][1], float(1), "queue two p1 correct wt");
    ASSERT_EQ(waiting_times[queue_name_2][2], float(2), "queue two p2 correct wt");

    ASSERT_LT(waiting_times[SimulationRunStats::all_queues()][SimulationRunStats::all_priorities()] - float(10.0/3), float(.01), "all qs avg correct wt");
    ASSERT_EQ(waiting_times[SimulationRunStats::all_queues()][1], float(3), "all qs p1 correct wt");
    ASSERT_EQ(waiting_times[SimulationRunStats::all_queues()][2], float(4), "all qs p2 correct wt");

    // in terms of CLR, things are more obvious
    ASSERT_EQ(clrs[queue_name_1][SimulationRunStats::all_priorities()], float(1.0/4), "queue one avg correct clr");
    ASSERT_EQ(clrs[queue_name_1][1], float(1.0/3), "queue one p1 correct clr");
    ASSERT_EQ(clrs[queue_name_1][2], float(0), "queue one p2 correct clr");

    ASSERT_EQ(clrs[queue_name_2][SimulationRunStats::all_priorities()], float(0), "queue two avg correct clr");
    ASSERT_EQ(clrs[queue_name_2][1], float(0), "queue two p1 correct clr");
    ASSERT_EQ(clrs[queue_name_2][2], float(0), "queue two p2 correct clr");

    ASSERT_EQ(clrs[SimulationRunStats::all_queues()][SimulationRunStats::all_priorities()], float(1.0/4), "all qs avg correct clr");
    ASSERT_EQ(clrs[SimulationRunStats::all_queues()][1], float(1.0/3), "all qs p1 correct clr");
    ASSERT_EQ(clrs[SimulationRunStats::all_queues()][2], float(0), "all qs p2 correct clr");

}

void test_spy_odd_entrances()
{
    auto customer = make_customer(1, 0, 1);
    auto customer2 = make_customer(2, 0, 1);
    auto customer3 = make_customer(3, 0, 1);
    customer->set_serviced(true);
    customer2->set_serviced(true);
    customer3->set_serviced(true);

    constexpr float kEnteredQueue = 1.0;
    constexpr float kExitedQueue = 2.0;
    constexpr float kExitedQueueLong = 3.0;

    std::string queue_name_1("queue1");
    std::string queue_name_2("queue2");

    // Customer 1 enters queue1 twice, taking 1 second each
    // Customer 1 enters queue2 once, taking 1 second
    // Customer 1 has 3s of total waiting time

    customer->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kEnteredQueue));
    customer->add_event(CustomerEvent(CustomerEventType::EXITED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kExitedQueue));

    customer->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::QUEUE,
                                      queue_name_2,
                                      kEnteredQueue));

    customer->add_event(CustomerEvent(CustomerEventType::EXITED,
                                      PlaceType::QUEUE,
                                      queue_name_2,
                                      kExitedQueue));

    customer->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kEnteredQueue));

    customer->add_event(CustomerEvent(CustomerEventType::EXITED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kExitedQueue));

    // Customer 2 enters queue1 twice, taking 1 second each
    // Customer 2 enters queue2 once, taking 1 second
    // Customer 2 has 3s of total waiting time

    customer2->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kEnteredQueue));
    customer2->add_event(CustomerEvent(CustomerEventType::EXITED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kExitedQueue));

    customer2->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::QUEUE,
                                      queue_name_2,
                                      kEnteredQueue));

    customer2->add_event(CustomerEvent(CustomerEventType::EXITED,
                                      PlaceType::QUEUE,
                                      queue_name_2,
                                      kExitedQueue));

    customer2->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kEnteredQueue));

    customer2->add_event(CustomerEvent(CustomerEventType::EXITED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kExitedQueue));

    // Customer 3 enters queue1 once, taking one second
    customer3->add_event(CustomerEvent(CustomerEventType::ENTERED,
                                      PlaceType::QUEUE,
                                      queue_name_1,
                                      kEnteredQueue));
    customer3->add_event(CustomerEvent(CustomerEventType::EXITED,
                                       PlaceType::QUEUE,
                                       queue_name_1,
                                       kExitedQueue));

    constexpr auto kNoLVal = 0;
    constexpr auto kMaxSize = 100;
    constexpr auto kTransientPeriod = 1000;
    SimulationSpy spy(kNoLVal,
                      kMaxSize,
                      {queue_name_1, queue_name_2},
                      kTransientPeriod);

    spy.on_customer_entering(customer);
    spy.on_customer_exiting(customer);
    spy.on_customer_entering(customer2);
    spy.on_customer_exiting(customer2);
    spy.on_customer_entering(customer3);
    spy.on_customer_exiting(customer3);

    queue_name_to_priority_to_stat waiting_times = spy.average_waiting_times();
    queue_name_to_priority_to_stat clrs = spy.customer_loss_rates();

    if (constants::DEBUG_ENABLED) {
        for (const auto & name_and_waiting_time_map : waiting_times) {
            std::cout << name_and_waiting_time_map.first << std::endl;
            for (const auto & priority_and_waiting_time : name_and_waiting_time_map.second) {
                std::cout << "    Priority-" << priority_and_waiting_time.first
                        << ": waiting_time = " << priority_and_waiting_time.second
                        << std::endl;
            }
        }

        for (const auto & name_and_waiting_clr_map : clrs) {
            std::cout << name_and_waiting_clr_map.first << std::endl;
            for (const auto & priority_and_clr : name_and_waiting_clr_map.second) {
                std::cout << "    Priority-" << priority_and_clr.first
                        << ": clr = " << priority_and_clr.second
                        << std::endl;
            }
        }
    }

    // RECAP
    // Customer 1 enters queue1 twice, taking 1 second each
    // Customer 1 enters queue2 once, taking 1 second
    // Customer 1 has 3s of total waiting time
    // Customer 2 enters queue1 twice, taking 1 second each
    // Customer 2 enters queue2 once, taking 1 second
    // Customer 2 has 3s of total waiting time
    // Customer 3 enters queue1 once, taking 1 second each

    ASSERT_EQ(waiting_times[queue_name_1][SimulationRunStats::all_priorities()], float(1), "queue one avg correct wt");
    ASSERT_EQ(waiting_times[queue_name_1][1], float(1), "queue one p1 correct wt");

    ASSERT_EQ(waiting_times[queue_name_2][SimulationRunStats::all_priorities()], float(1), "queue two avg correct wt");
    ASSERT_EQ(waiting_times[queue_name_2][1], float(1), "queue two p1 correct wt");

    ASSERT_EQ(waiting_times[SimulationRunStats::all_queues()][SimulationRunStats::all_priorities()], float(7.0/3), "all qs avg correct wt");
    ASSERT_EQ(waiting_times[SimulationRunStats::all_queues()][1], float(7.0/3), "all qs p1 correct wt");
}

void test_bounded_pareto()
{
    constexpr double kLowerBound = 332;
    constexpr double kUpperBound = 1e10;
    constexpr double kAlpha = 1.1;
    BoundedParetoGenerator generator(kLowerBound, kUpperBound, kAlpha);

    constexpr auto kNumbersToGenerate = 10'000'000;
    double min = INFINITY;
    double max = 0;
    long double mean = 0;

    for (auto i = 0; i < kNumbersToGenerate; ++i) {
        auto generated_number = generator.generate();

        if (generated_number < min) {
            min = generated_number;
        }
        if (generated_number > max) {
            max = generated_number;
        }

        mean += generated_number / kNumbersToGenerate;
    }

    ASSERT_GT(min, double(331.9999));
    ASSERT_LT(max, double(1e10));
    ASSERT_GT(mean, double(2800));
    ASSERT_LT(mean, double(3200));
}

} // testing
