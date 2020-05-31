#pragma once

namespace testing {

void ASSERT(bool condition, const std::string & explanation = "");

void run_all_tests();

// tests

void test_tests();

} // testing
