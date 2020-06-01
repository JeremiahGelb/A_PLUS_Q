

namespace prng {

class ExponentialGenerator {
public:
    ExponentialGenerator(long seed = 0)
    : seed_(seed)
    {}

    float generate();
private:
    long seed_;
};

} // namespace prng