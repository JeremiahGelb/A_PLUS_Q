#include <math.h>
#pragma once

template <class T>
class RandomNumberGenerator {
public:
    RandomNumberGenerator(long seed = 0)
    : seed_(seed)
    {}
    virtual T generate() const = 0;
protected:
    mutable long seed_;
};

class ExponentialGenerator : public RandomNumberGenerator<float> {
public:
    ExponentialGenerator(float lambda, long seed = 0)
    : RandomNumberGenerator(seed)
    , one_over_lambda_(1/lambda)
    {}

    float generate() const override;
private:
    float one_over_lambda_;
};

class UniformGenerator : public RandomNumberGenerator<float> {
public:
    UniformGenerator(long seed = 0)
    : RandomNumberGenerator(seed)
    {}

    float generate() const override;
};

class BoundedParetoGenerator : public RandomNumberGenerator<double> {
public:
    BoundedParetoGenerator(double lower_bound,
                           double upper_bound,
                           double alpha,
                           long seed = 0)
    : RandomNumberGenerator(seed)
    , h_to_the_alpha_(pow(upper_bound, alpha))
    , l_to_the_alpha_(pow(lower_bound, alpha))
    , l_to_the_alpha_times_h_to_the_alpha_(h_to_the_alpha_*l_to_the_alpha_)
    , l_to_the_alpha_plus_h_to_the_alpha_(h_to_the_alpha_+l_to_the_alpha_)
    , negative_one_over_alpha_(-1.0/alpha)
    {}

    double generate() const override;
private:
    double h_to_the_alpha_;
    double l_to_the_alpha_;
    double l_to_the_alpha_times_h_to_the_alpha_;
    double l_to_the_alpha_plus_h_to_the_alpha_;
    double negative_one_over_alpha_;
};
