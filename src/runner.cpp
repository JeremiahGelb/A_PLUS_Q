
#include <iostream>
#include <vector>
#include "test.h"

void run_tests() {
    testing::run_all_tests();
}

int main(int argc, char ** argv) {
    std::vector<std::string> args;

    for (auto i = 0; i < argc; ++i) {
        args.push_back(std::string(argv[i]));
    }

    if (args.size() == 1) {
        std::cout << "You forgot the args" << std::endl;
    }

    for (const auto & arg : args) {
        if (arg == "--test") {
            run_tests();
            return 0;
        }
    }
}
