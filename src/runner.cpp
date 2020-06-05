
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>

#include "test.h"
#include "proj_1.h"

int main(int argc, char ** argv) {

    // not needed but I don't want to work with char ** :)
    std::vector<std::string> args;
    for (auto i = 0; i < argc; ++i) {
        args.push_back(std::string(argv[i]));
    }

    constexpr auto kTestArgsNumber = 2; // program name and --test
    constexpr auto kRunArgsNumber = 5; // pname, Lambda, K, C, L
    const auto received_params = args.size();
    if (!(received_params == kTestArgsNumber || received_params == kRunArgsNumber)) {
        std::cout << "Invalid Number of Args" << std::endl;
        return 0;
    }

    constexpr auto kLambdaOrTestIndex = 1;
    constexpr auto kQueueSizeIndex = 2;
    constexpr auto kCustomersToServeIndex = 3;
    constexpr auto kLIndex = 4;

    if (args[kLambdaOrTestIndex] == "--test") {
        testing::run_all_tests();
        return 0;
    }

    float lambda = 0;
    std::size_t queue_size = 0;
    std::size_t customers_to_serve = 0;
    std::size_t L = 0;

    std::stringstream(args[kLambdaOrTestIndex]) >> lambda;
    std::stringstream(args[kQueueSizeIndex]) >> queue_size;
    std::stringstream(args[kCustomersToServeIndex]) >> customers_to_serve;
    std::stringstream(args[kLIndex]) >> L;

    if (lambda == 0
        || queue_size == 0
        || customers_to_serve == 0
        || L == 0) {
        std::cout << "Invalid Args" << std::endl;
        return 0;
    }

    auto start = std::chrono::high_resolution_clock::now();
    run_project_1(lambda, queue_size, customers_to_serve, L);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << customers_to_serve  << " customers took "
              << duration.count() << " milliseconds!" << std::endl; 

}
