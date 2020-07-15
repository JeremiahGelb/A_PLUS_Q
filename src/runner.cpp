
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>

#include "test.h"
#include "proj_1.h"
#include "proj_2.h"
#include "proj_3.h"

void print_help_text(std::string_view error = "")
{
    if (!error.empty()) {
        std::cout << error << std::endl;
    }
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
        print_help_text("Invalid Args for proj1");
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();
    run_project_1(lambda, queue_size, customers_to_serve, L);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << customers_to_serve  << " customers took "
              << duration.count() << " milliseconds!" << std::endl;
}

void proj_2(const std::vector<std::string> & args)
{
    // pname, mode, lambda, kcpu, kio, C, L, M
    constexpr auto kLambdaIndex = 2;
    constexpr auto kCpuQueueSizeIndex = 3;
    constexpr auto kIoQueueSizeIndex = 4;
    constexpr auto kCustomersToServeIndex = 5;
    constexpr auto kLIndex = 6;
    constexpr auto kMIndex = 7;

    float lambda = 0;
    std::size_t cpu_queue_size = 0;
    std::size_t io_queue_size = 0;
    std::size_t customers_to_serve = 0;
    std::size_t L = 100;
    std::size_t M = 0;

    std::stringstream(args[kLambdaIndex]) >> lambda;
    std::stringstream(args[kCpuQueueSizeIndex]) >> cpu_queue_size;
    std::stringstream(args[kIoQueueSizeIndex]) >> io_queue_size;
    std::stringstream(args[kCustomersToServeIndex]) >> customers_to_serve;
    std::stringstream(args[kLIndex]) >> L;
    std::stringstream(args[kMIndex]) >> M;

    if (lambda == 0
        || cpu_queue_size == 0
        || customers_to_serve == 0
        || io_queue_size == 0
        || L == 100
        || M == 0) {
        print_help_text("Error Parsing Args for proj2");
        return;
    }

    project2::Mode mode;
    switch (L) {
    case 0:
        mode = project2::Mode::MM1;
        break;
    case 1:
        mode = project2::Mode::CPU;
        break;
    default:
        print_help_text("Invalid parameter L for project 2 -> expexted 0 for MM1 or 1 for CPU");
        return;
    }

    project2::Discipline discipline;
    switch(M) {
    case 1:
        discipline = project2::Discipline::FCFS;
        break;
    case 2:
        discipline = project2::Discipline::LCFS_NP;
        break;
    case 3:
        discipline = project2::Discipline::SJF_NP;
        break;
    case 4:
        discipline = project2::Discipline::PRIO_NP;
        break;
    case 5:
        discipline = project2::Discipline::PRIO_P;
        break;
    default:
        print_help_text("Invalid M for project 2 [1-5] for fcfs, lcfs_np, sjf_np, prio_np, prio_preempt");
        return;
    }

    constexpr size_t kRuns = 30; // number of runs to generate stats from
    auto start = std::chrono::high_resolution_clock::now();

    project2::run_project_2(lambda,
                            cpu_queue_size,
                            io_queue_size,
                            customers_to_serve,
                            mode,
                            discipline,
                            kRuns);

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << kRuns << " runs of "
              << customers_to_serve  << " customers took "
              << duration.count() << " milliseconds!" << std::endl;
}

void proj_3(const std::vector<std::string> & args)
{
    // pname, lambda, C, L, M
    constexpr auto kLambdaIndex = 2;
    constexpr auto kCustomersToServeIndex = 3;
    constexpr auto kLIndex = 4;
    constexpr auto kMIndex = 5;

    float lambda = 0;
    std::size_t customers_to_serve = 0;
    std::size_t L = 0;
    std::size_t M = 100;

    std::stringstream(args[kLambdaIndex]) >> lambda;
    std::stringstream(args[kCustomersToServeIndex]) >> customers_to_serve;
    std::stringstream(args[kLIndex]) >> L;
    std::stringstream(args[kMIndex]) >> M;

    if (lambda == 0
        || customers_to_serve == 0
        || L == 0
        || M == 100) {
        std::cout << "Error Parsing Args for proj3" << std::endl;
        print_help_text();
        return;
    }

    project3::Discipline discipline;
    switch(L) {
    case 1:
        discipline = project3::Discipline::FCFS;
        break;
    case 2:
        discipline = project3::Discipline::SJF_NP;
        break;
    default:
        print_help_text("Unknown L in project 3, expected 1,2 for FCFS, SJF_NP");
        return;
    }

    project3::Mode mode;
    switch (M) {
    case 0:
        mode = project3::Mode::MM3;
        break;
    case 1:
        mode = project3::Mode::MG3;
        break;
    case 2:
        mode = project3::Mode::MG1;
        break;
    default:
        print_help_text("Unknown M in project 3, expected 0,1,2 for MM3, MG3, MG1");
        return;
    }


    constexpr size_t kRuns = 30; // number of runs to generate stats from
    auto start = std::chrono::high_resolution_clock::now();

    project3::run_project_3(lambda,
                            customers_to_serve,
                            discipline,
                            mode,
                            kRuns);

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << kRuns << " runs of "
              << customers_to_serve  << " customers took "
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
        proj_2(args);
    } else if (mode == "--proj3") {
        if (received_params != kProj3ArgsNumber) {
            print_help_text();
            return 0;
        }
        proj_3(args);
    } else {
        print_help_text();
    }

    return 0;
}
