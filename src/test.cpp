#include <vector>
#include <functional>
#include <utility>
#include <string>
#include <iostream>

#include "test.h"
#include "colormod.h"
#include "simulation_timer.h"

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
        std::make_pair("Simulation Timer", test_simulation_timer)
    };


    for (const auto & test : tests) {
        try {
            std::cout << std::endl <<  "Starting test: " << test.first << std::endl;
            test.second();
            std::cout << green << "Passed test: " << def << test.first << std::endl;
        } catch (TestFailure & e) {
            std::cout << red <<  "Failed test: " << def <<  test.first << std::endl;
            std::cout << "explanation: " << e.what() << std::endl;
        } catch (std::exception & e) {
            std::cout << red <<  "Caught unhandled exception in test: " << def <<  test.first << std::endl;
            std::cout << "explanation: " << e.what() << std::endl;           
        }
    }

    std::cout << std::endl;
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

    timer.register_job(timer.time() + 1, [&call_order] {call_order.push_back(1);});
    timer.register_job(timer.time() + 3, [&call_order] {call_order.push_back(4);});
    timer.register_job(timer.time() + 2, [&call_order] {call_order.push_back(2);});
    timer.register_job(timer.time() + 2, [&call_order] {call_order.push_back(3);});

    ASSERT(call_order.empty());

    timer.advance_time();
    ASSERT(call_order.size() == 1, "first call did first job");

    timer.advance_time();
    ASSERT(call_order.size() == 3, "second call did two jobs");

    timer.advance_time();
    ASSERT(call_order.size() == 4, "third call did last job");

    for (std::uint32_t i = 0; i < 4; ++i) {
        ASSERT(call_order[i] = i+1, "Call order was correct");
    }

    try {
        timer.advance_time();
        ASSERT(false, "expected to throw runtime error");
    } catch(std::runtime_error &) {}

}

} // testing