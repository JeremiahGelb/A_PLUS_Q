#pragma once

#include <cstdint>
#include <cstddef>

#include "stats.h"

namespace project3 {

enum class Mode {
    MM3,
    MG3,
    MG1
};

enum class Discipline {
    FCFS,
    SJF_NP,
};

void run_project_3(float lambda,
                   std::size_t customers_to_serve,
                   Discipline discipline,
                   Mode mode,
                   const int runs);

SimulationRunStats do_one_run(float lambda,
                              std::size_t customers_to_serve,
                              Discipline discipline,
                              Mode mode,
                              long seed_offset);

} // project3
