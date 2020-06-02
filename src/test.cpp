#include <vector>
#include <functional>
#include <utility>
#include <string>
#include <iostream>

#include "test.h"
#include "colormod.h"
#include "simulation_timer.h"
#include "customer.h"
#include "prng.h"
#include "incoming_customers.h"
#include "queue.h"

namespace {
    Color::Modifier red(Color::FG_RED);
    Color::Modifier green(Color::FG_GREEN);
    Color::Modifier def(Color::FG_DEFAULT);
} // anonymous

namespace testing {

void ASSERT(bool condition, const std::string & explanation)
{
    if (!condition) {
        throw(TestFailure(explanation));
    }
}

void run_all_tests()
{

    std::vector<std::pair<std::string, std::function<void()>>> tests = {
        std::make_pair("Blank", [] {}),
        std::make_pair("Test", test_tests),
        std::make_pair("Simulation Timer", test_simulation_timer),
        std::make_pair("Customer", test_customer),
        std::make_pair("prng", test_prng),
        std::make_pair("incoming customers", test_incoming_customers),
        std::make_pair("queue", test_queue)
    };

    auto failure_count = 0;
    auto unhandled_exception_count = 0;
    for (const auto & test : tests) {
        try {
            std::cout << std::endl <<  "Starting test: " << test.first << std::endl;
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

    constexpr auto kExpectedFailures = 1; // test_tests
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
    ASSERT(false, "this test failing is good!");
}

void test_simulation_timer()
{
    SimulationTimer timer;
    ASSERT(timer.time() == 0, "initial time wasn't 0");

    std::vector<std::uint32_t> call_order;

    timer.register_job(timer.time() + 1.1, [&call_order] {call_order.push_back(1);});
    timer.register_job(timer.time() + 3.0, [&call_order] {call_order.push_back(4);});
    timer.register_job(timer.time() + 2.2, [&call_order] {call_order.push_back(2);});
    timer.register_job(timer.time() + 2.2, [&call_order] {call_order.push_back(3);});

    ASSERT(call_order.empty());

    timer.advance_time();
    ASSERT(call_order.size() == 1, "first call did first job");

    timer.advance_time();
    ASSERT(call_order.size() == 3, "second call did two jobs");

    timer.advance_time();
    ASSERT(call_order.size() == 4, "third call did last job");

    for (std::uint32_t i = 0; i < 4; ++i) {
        ASSERT(call_order[i] == i+1, "Call order was correct");
    }

    try {
        timer.advance_time();
        ASSERT(false, "expected to throw runtime error");
    } catch(std::runtime_error &) {}

}

void test_customer()
{
    constexpr auto kFirstCustomerId = 1;
    constexpr auto kFirstCustomerArrivalTime = 2.0;
    constexpr auto kFirstCustomerServiceTime = 3.0;

    constexpr auto kDefaultServiced = false;
    constexpr auto kDefaultDepartureTime = 0;

    auto first_customer = make_customer(kFirstCustomerId,
                                                  kFirstCustomerArrivalTime,
                                                  kFirstCustomerServiceTime);

    ASSERT(first_customer->id() == kFirstCustomerId, "initial id matches");
    ASSERT(first_customer->arrival_time() == kFirstCustomerArrivalTime, "initial arrival time matches");
    ASSERT(first_customer->service_time() == kFirstCustomerServiceTime, "initial service time matches");
    ASSERT(first_customer->serviced() == kDefaultServiced, "initial serviced matches");
    ASSERT(first_customer->departure_time() == kDefaultDepartureTime, "initial departure time matches");

    constexpr auto new_serviced = true;
    constexpr auto new_departure_time = 10.0;

    first_customer->set_serviced(new_serviced);
    first_customer->set_departure_time(new_departure_time);

    ASSERT(first_customer->serviced() == new_serviced, "new serviced matches");
    ASSERT(first_customer->departure_time() == new_departure_time, "new departure time matches");

    auto first_customer_string = first_customer->to_string();
    const std::string kExpectedString = "1,2,3,1,10\n";
    ASSERT(first_customer_string == kExpectedString, "to string works as expected");

    auto second_customer = make_customer(kExpectedString);
    ASSERT(second_customer->id() == kFirstCustomerId, "second_customer id matches");
    ASSERT(second_customer->arrival_time() == kFirstCustomerArrivalTime, "second_customer arrival time matches");
    ASSERT(second_customer->service_time() == kFirstCustomerServiceTime, "second_customer service time matches");
    ASSERT(second_customer->serviced() == new_serviced, "second_customer serviced matches");
    ASSERT(second_customer->departure_time() == new_departure_time, "second_customer departure time matches");

    ASSERT(*first_customer == *second_customer, "customer == works");
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
    
    ASSERT(first_default_float != second_default_float,
           "sequantial generations shouldn't be equal");

    ASSERT(first_seeded_float == second_seeded_float,
           "Same seed = same number");

    auto small_lambda_sum = 0;
    auto big_lambda_sum = 0;
    for (auto i = 0; i < 30 ; ++i) {
        small_lambda_sum += seeded_exp_gen.generate();
        big_lambda_sum += bigger_lambda_exp_gen.generate();
    }

    ASSERT(small_lambda_sum > big_lambda_sum, "large lambda produces small values");
}

void test_incoming_customers()
{
    auto customer_list = std::vector<std::shared_ptr<Customer>>();

    auto customer_callback = [&customer_list] (std::shared_ptr<Customer> customer) {
        customer_list.push_back(customer);
    };

    SimulationTimer timer;

    auto incoming_customers = IncomingCustomers<ExponentialGenerator,
                                                ExponentialGenerator>(timer,
                                                                      ExponentialGenerator(1, 0),
                                                                      ExponentialGenerator(1, 10));

    incoming_customers.register_for_customers(customer_callback);
    ASSERT(customer_list.size() == 0, "empty before start");

    incoming_customers.start();
    ASSERT(customer_list.size() == 0, "empty before run");

    timer.advance_time();
    ASSERT(customer_list.size() == 1, "first customer added");

    timer.advance_time();
    ASSERT(customer_list.size() == 2, "second customer added");

    // add some more customers
    timer.advance_time();
    timer.advance_time();
    timer.advance_time();
    timer.advance_time();
    timer.advance_time();

    float last_arrival_time = 0.0;
    float last_service_time = 0.0;

    for (std::uint32_t i = 0; i < customer_list.size(); i++) {
        auto & customer = customer_list[i];
        ASSERT(customer->id() == i, "ids are sequential");

        auto arrival_time = customer->arrival_time();
        ASSERT(arrival_time > last_arrival_time, "times are increasing");
        last_arrival_time = arrival_time;

        auto service_time = customer->service_time();
        ASSERT(service_time != last_service_time, "service times are different");
        last_service_time = service_time;
    }

}

void test_queue()
{
    constexpr auto kMaxSize = 10;
    auto queue = Queue(kMaxSize);

    ASSERT(queue.size() == 0, "queue empty at start");

    auto insert = queue.accept_customer_callback();

    std::vector<std::shared_ptr<Customer>> received_customers;

    auto request = [&received_customers] (std::shared_ptr<Customer> customer) {
        received_customers.push_back(customer);
    };

    // testing basic functionality
    insert(make_customer(0, 1.1, 1.1));
    ASSERT(queue.size() == 1, "queue now has one customer");

    queue.request_one_customer(request);
    ASSERT(queue.size() == 0, "queue empty again");
    ASSERT(received_customers.size() == 1, "customer made it to vector");

    queue.request_one_customer(request);
    queue.request_one_customer(request);
    insert(make_customer(1, 1.1, 1.1));
    ASSERT(queue.size() == 0, "first pending request fulfilled");
    insert(make_customer(2, 1.1, 1.1));
    ASSERT(queue.size() == 0, "second pending request fulfilled");
    ASSERT(received_customers.size() == 3, "got all those customers");


    // testing customers are delivered FIFO
    received_customers.clear();
    ASSERT(received_customers.size() == 0, "clear");
    insert(make_customer(0, 1.1, 1.1));
    insert(make_customer(1, 1.1, 1.1));
    insert(make_customer(2, 1.1, 1.1));
    queue.request_one_customer(request);
    queue.request_one_customer(request);
    queue.request_one_customer(request);
    std::uint32_t counter = 0;
    for (const auto & customer : received_customers) {
        ASSERT(customer->id() == counter, "customers are fifo");
        ++counter;
    }

    // testing requests are handled FIFO
    std::vector<int> call_order;
    auto request_1 = [&call_order] (std::shared_ptr<Customer>) {
        call_order.push_back(0);
    };
    auto request_2 = [&call_order] (std::shared_ptr<Customer>) {
        call_order.push_back(1);
    };
    queue.request_one_customer(request_1);
    queue.request_one_customer(request_2);
    insert(make_customer(0, 1.1, 1.1));
    insert(make_customer(1, 1.1, 1.1));
    ASSERT(call_order.size() == 2, "both got handled");
    ASSERT(call_order[0] == 0, "first handled first");
    ASSERT(call_order[1] == 1, "second handled second");

    // testing max size
    ASSERT(queue.size() == 0, "queue empty before testing max");
    for(auto i = 0; i < kMaxSize; i++) {
        insert(make_customer(0, 1.1, 1.1));
    }
    ASSERT(queue.size() == kMaxSize, "queue is at max");
    insert(make_customer(0, 1.1, 1.1));
    ASSERT(queue.size() == kMaxSize, "queue didn't excede max");

}

} // testing