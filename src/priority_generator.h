#pragma once

#include  <cstdint>

#include "prng.h"

class UniformPriorityGenerator {
public:
    UniformPriorityGenerator(std::uint32_t min,
                             std::uint32_t max,
                             long seed)
    : min_(min)
    , max_(max)
    , generator_(UniformGenerator(seed))
    {
    }

    std::uint32_t generate()
    {
        auto zero_to_one = generator_.generate();
        auto min_to_max_plus_one =  (max_ - min_ + 1) * zero_to_one + min_;
        auto priority = std::uint32_t(min_to_max_plus_one); // round down
        if (priority > max_) {
            priority = max_; // incase zero_to_one == 1.0
        }
        return  priority;
    }
private:
    const std::uint32_t min_;
    const std::uint32_t max_;
    UniformGenerator generator_;
};