#pragma once

class RandomNumberGenerator {
public:
    RandomNumberGenerator(long seed = 0)
    : seed_(seed)
    {}
    virtual float generate() const = 0;
protected:
    mutable long seed_;
};

class ExponentialGenerator : public RandomNumberGenerator {
public:
    ExponentialGenerator(float lambda, long seed = 0)
    : RandomNumberGenerator(seed)
    , one_over_lambda_(1/lambda)
    {}

    float generate() const override;
private:
    float one_over_lambda_;
};

class UniformGenerator : public RandomNumberGenerator {
public:
    UniformGenerator(long seed = 0)
    : RandomNumberGenerator(seed)
    {}

    float generate() const override;
};
