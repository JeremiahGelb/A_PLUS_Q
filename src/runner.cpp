
#include <iostream>
#include <vector>
#include "test.h"

void run_tests() {
    std::cout << "Starting Tests" << std::endl;
    testing::run_all_tests();
    std::cout << "Testing complete" << std::endl;
}

int main(int argc, char ** argv) {
    std::vector<std::string> args;

    for (auto i = 0; i < argc; ++i) {
        args.push_back(std::string(argv[i]));
    }

    for (const auto & arg : args) {
        if (arg == "--test") {
            run_tests();
            return 0;
        }
    }
}
