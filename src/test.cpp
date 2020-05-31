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
        throw(std::runtime_error(explanation));
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
        } catch (std::exception & e) {
            std::cout << red <<  "Failed test: " << def <<  test.first << std::endl;
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
}

} // testing