#pragma once

namespace prng {

class RandomNumberGenerator {
public:
    RandomNumberGenerator(long seed = 0)
    : seed_(seed)
    {}
    virtual float generate() = 0;
protected:
    long seed_;
};

class ExponentialGenerator : public RandomNumberGenerator {
public:
    ExponentialGenerator(long seed = 0)
    : RandomNumberGenerator(seed)
    {}

    float generate() override;
};

} // namespace prng