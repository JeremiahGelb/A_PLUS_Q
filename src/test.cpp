#include <vector>
#include <functional>
#include <utility>
#include <string>
#include <iostream>

#include "test.h"
#include "simulation_timer.h"
#include "customer.h"
#include "prng.h"
#include "incoming_customers.h"
#include "queue.h"
#include "server.h"

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
        std::make_pair("Server", test_server)
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

    constexpr auto kDefaultServiced = false;
    constexpr float kDefaultServiceTime = 0;
    constexpr float kDefaultDepartureTime = 0;

    auto first_customer = make_customer(kFirstCustomerId, kFirstCustomerArrivalTime);

    ASSERT_EQ(first_customer->id(), kFirstCustomerId, "initial id matches");
    ASSERT_EQ(first_customer->arrival_time(), kFirstCustomerArrivalTime, "initial arrival time matches");
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
    const std::string kExpectedString = "1,2,1.234,1,10.5";
    ASSERT_EQ(first_customer_string, kExpectedString, "to string works as expected");

    auto second_customer = make_customer(kExpectedString);
    ASSERT_EQ(second_customer->id(), kFirstCustomerId, "second_customer id matches");
    ASSERT_EQ(second_customer->arrival_time(), kFirstCustomerArrivalTime, "second_customer arrival time matches");
    ASSERT_EQ(second_customer->service_time(), kNewServiceTime, "second_customer service time matches");
    ASSERT_EQ(second_customer->serviced(), kNewServiced, "second_customer serviced matches");
    ASSERT_EQ(second_customer->departure_time(), kNewDepartureTime, "second_customer departure time matches");

    ASSERT_EQ(*first_customer, *second_customer, "customer == works");
}

void test_prng()
{
    constexpr long kSeed = 12345;
    constexpr float kLambda = 1;
    constexpr long kArbitratyOffset = 1000;

    ExponentialGenerator default_exp_gen(kLambda);
    ExponentialGenerator seeded_exp_gen(kLambda, kSeed);
    ExponentialGenerator same_seeded_exp_gen(kLambda, kSeed);

    ExponentialGenerator bigger_lambda_exp_gen(10*kLambda, kSeed + kArbitratyOffset);

    auto first_default_float = default_exp_gen.generate();
    auto second_default_float = default_exp_gen.generate();

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
    auto queue = Queue(kMaxSize, exit_customer, ExponentialGenerator(kLambda));

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
    for(std::size_t i = 0; i < kMaxSize; i++) {
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
                       ExponentialGenerator(kLambda),
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
    for(std::size_t i = 0; i < kMaxSize; i++) {
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
                       ExponentialGenerator(kLambda),
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
    for(std::size_t i = 0; i < kMaxSize; i++) {
        insert(make_customer(0, 1.1));
    }
    
    ASSERT_EQ(rejected_customers.size(), std::size_t(0), "no rejected customers");
    ASSERT_EQ(queue.size(), kMaxSize, "queue is at max");
    insert(make_customer(0, 1.1));
    ASSERT_EQ(queue.size(), kMaxSize, "queue didn't excede max");
    ASSERT_EQ(rejected_customers.size(), std::size_t(1), "one rejected customer");
}

void test_server()
{
    SimulationTimer timer;

    constexpr auto kId = 0;
    constexpr auto kArrivalTime = 1;
    constexpr auto kServiceTime = 2;

    auto call_count = 0;
    CustomerRequestHandler customer_request_handler = [&call_count] (const CustomerRequest & request) {
        auto customer = make_customer(kId, kArrivalTime);
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
}

} // testing