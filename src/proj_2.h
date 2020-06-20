#pragma once

#include <cstdint>
#include <cstddef>

#include "stats.h"

namespace project2 {

enum class Mode {
    MM1,
    CPU
};

enum class Discipline {
    FCFS,
    LCFS_NP,
    SJF_NP,
    PRIO_NP,
    PRIO_P
};

void run_project_2(float lambda,
                   std::size_t max_cpu_queue_customers,
                   std::size_t max_io_queue_customers,
                   std::size_t customers_to_serve,
                   Mode mode,
                   Discipline discipline,
                   const int runs);


SimulationRunStats do_one_run(float lambda,
                              std::size_t max_cpu_queue_customers,
                              std::size_t max_io_queue_customers,
                              std::size_t customers_to_serve,
                              Mode mode,
                              Discipline discipline,
                              long seed_offset);

}
