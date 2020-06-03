#pragma once

#include <string>

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


void ASSERT(bool condition, const std::string & explanation = "");

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
