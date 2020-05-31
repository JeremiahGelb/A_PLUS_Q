#include <vector>
#include <functional>
#include <utility>
#include <string>
#include <iostream>

#include "test.h"

namespace testing {
void ASSERT(bool condition, const std::string & explanation) {
    if (!condition) {
        throw(std::runtime_error(explanation));
    }
}

void run_all_tests() {

    std::vector<std::pair<std::string, std::function<void()>>> tests = {
        std::make_pair("Blank Test", [] {})
    };


    for (const auto & test : tests) {
        try {
            std::cout << "Starting test: " << test.first << std::endl;
            test.second();
            std::cout << "Passed test: " << test.first << std::endl;
        } catch (std::exception & e) {
            std::cout << "failed test: " << test.first << std::endl;
            std::cout << e.what() << std::endl;
        }
    }
}

} // testing