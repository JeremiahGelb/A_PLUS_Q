
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>

#include "test.h"
#include "proj_1.h"

void print_help_text()
{
    std::cout << "try one of these options:" << std::endl;
    std::cout << "1) ./run.o --test" << std::endl;
    std::cout << "2) ./run.o --proj1 Lambda K C L)" << std::endl;
    std::cout << "3) ./run.o --proj2 Lambda Kcpu Kio C L M" << std::endl;
    std::cout << "4) ./run.o --proj3 Lambda C L M" << std::endl;
}

void proj_1(const std::vector<std::string> & args)
{
    constexpr auto kLambdaIndex = 2;
    constexpr auto kQueueSizeIndex = 3;
    constexpr auto kCustomersToServeIndex = 4;
    constexpr auto kLIndex = 5;

    float lambda = 0;
    std::size_t queue_size = 0;
    std::size_t customers_to_serve = 0;
    std::size_t L = 0;

    std::stringstream(args[kLambdaIndex]) >> lambda;
    std::stringstream(args[kQueueSizeIndex]) >> queue_size;
    std::stringstream(args[kCustomersToServeIndex]) >> customers_to_serve;
    std::stringstream(args[kLIndex]) >> L;

    if (lambda == 0
        || queue_size == 0
        || customers_to_serve == 0
        || L == 0) {
        std::cout << "Invalid Args for proj1" << std::endl;
        print_help_text();
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();
    run_project_1(lambda, queue_size, customers_to_serve, L);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << customers_to_serve  << " customers took "
              << duration.count() << " milliseconds!" << std::endl; 
}

int main(int argc, char ** argv) {

    // not needed but I don't want to work with char ** :)
    std::vector<std::string> args;
    for (auto i = 0; i < argc; ++i) {
        args.push_back(std::string(argv[i]));
    }

    constexpr auto kTestArgsNumber = 2; // program name and --test
    constexpr auto kProj1ArgsNumber = 6; // pname, mode, Lambda, K, C, L
    constexpr auto kProj2ArgsNumber = 8; // pname, mode, lambda, kcpu, kio, C, L, M
    constexpr auto kProj3ArgsNumber = 6; // pname, mode, lambda, C, L, M
    const auto received_params = args.size();

    constexpr auto kModeIndex = 1;

    const auto & mode = args[kModeIndex];
    if (mode == "--test") {
        if (received_params != kTestArgsNumber) {
            print_help_text();
            return 0;
        }
        testing::run_all_tests();
    } else if (mode == "--proj1") {
        if (received_params != kProj1ArgsNumber) {
            print_help_text();
            return 0;
        }
        proj_1(args);
    } else if (mode == "--proj2") {
        if (received_params != kProj2ArgsNumber) {
            print_help_text();
            return 0;
        }
        std::cout << "TODO" << std::endl;
    } else if (mode == "--proj3") {
        if (received_params != kProj3ArgsNumber) {
            print_help_text();
            return 0;
        }
        std::cout << "TODO" << std::endl;
    } else {
        print_help_text();
    }
 
    return 0;
}
