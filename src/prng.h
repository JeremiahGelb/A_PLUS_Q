#pragma once

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
    ExponentialGenerator(float lambda, long seed = 0)
    : RandomNumberGenerator(seed)
    , one_over_lambda_(1/lambda)
    {}

    float generate() override;
private:
    float one_over_lambda_;
};
