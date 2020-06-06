#pragma once

#include <string>

#include "constants.h"
#include "colormod.h"

namespace {

Color::Modifier red(Color::FG_RED);
Color::Modifier green(Color::FG_GREEN);
Color::Modifier blue(Color::FG_BLUE);
Color::Modifier yellow(Color::FG_YELLOW);
Color::Modifier def(Color::FG_DEFAULT);

} // anon
namespace testing {

struct TestFailure : public std::exception
{
    TestFailure(const std::string & info)
    : info_(info)
    {
    }

    const std::string info_;
	const char * what () const throw ()
    {
    	return info_.c_str();
    }
};


inline void ASSERT(bool condition, const std::string & explanation = "")
{
    if (!condition) {
        throw(TestFailure(explanation));
    }
}

template<class L, class R>
inline void ASSERT_EQ(const L & lhs, const R & rhs, const std::string & explanation = "")
{
    if (constants::STRICT_TEST_TYPES) {
        if (typeid(R) != typeid(L)) {
            std::cout << yellow << "WARNING" << def
                      << " comparing " << typeid(L).name()
                      << " to " << typeid(R).name()
                      << " in assertion " << explanation << std::endl;
        }
    }
    ASSERT(lhs == rhs, explanation);
}

template<class L, class R>
inline void ASSERT_NEQ(const L & lhs, const R & rhs, const std::string & explanation = "")
{
    if (constants::STRICT_TEST_TYPES) {
        if (typeid(R) != typeid(L)) {
            std::cout << yellow << "WARNING" << def
                      << " comparing " << typeid(L).name()
                      << " to " << typeid(R).name()
                      << " in assertion " << explanation << std::endl;
        }
    }
    ASSERT(lhs != rhs, explanation);
}

template<class L, class R>
inline void ASSERT_LT(const L & lhs, const R & rhs, const std::string & explanation = "")
{
    if (constants::STRICT_TEST_TYPES) {
        if (typeid(R) != typeid(L)) {
            std::cout << yellow << "WARNING" << def
                      << " comparing " << typeid(L).name()
                      << " to " << typeid(R).name()
                      << " in assertion " << explanation << std::endl;
        }
    }
    ASSERT(lhs < rhs, explanation);
}

template<class L, class R>
inline void ASSERT_GT(const L & lhs, const R & rhs, const std::string & explanation = "")
{
    if (constants::STRICT_TEST_TYPES) {
        if (typeid(R) != typeid(L)) {
            std::cout << yellow << "WARNING" << def
                      << " comparing " << typeid(L).name()
                      << " to " << typeid(R).name()
                      << " in assertion " << explanation << std::endl;
        }
    }
    ASSERT(lhs > rhs, explanation);
}


void run_all_tests();

// tests
void test_tests();
void test_simulation_timer();
void test_customer();
void test_prng();
void test_incoming_customers();
void test_queue();
void test_server();

} // testing
