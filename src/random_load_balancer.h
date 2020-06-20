#pragma once

#include <utility>
#include <vector>

#include "customer.h"
#include "prng.h"

using RandomLoadBalancerTarget = std::pair<CustomerRequest, float>;
// Each target has the associated upper probability between 0 and 1
// then we randomly generate a number between 0 and 1 and select the correct target
// so if the targets are {A, .3}, {B, .5}, {C, 1.0}
// A has a 30% chance of being chosen, B has a 20%, and C has 50%
// if we generated the number .4, we would choose B

class RandomLoadBalancer {
public:
    RandomLoadBalancer(const std::vector<RandomLoadBalancerTarget> & targets,
                       const UniformGenerator & uniform_generator)
    : targets_(targets)
    , uniform_generator_(uniform_generator)
    {
        float upper = 0.0;

        for (const auto & target : targets_) {
            if (upper >= target.second) {
                throw std::invalid_argument("Upper probabilities must be in increasing order");
            }
            upper = target.second;
        }

        if (upper != 1.0) {
            throw std::invalid_argument("Last upper probability must be 1");
        }
    }

    void route_customer(const std::shared_ptr<Customer> & customer)
    {
        auto generated_number = uniform_generator_.generate();

        for (const auto & target : targets_) {
            if (generated_number < target.second) {
                target.first(customer);
                return;
            }
        }
    }

private:
    const std::vector<RandomLoadBalancerTarget> targets_;
    UniformGenerator uniform_generator_;
};