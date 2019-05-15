#pragma once

#include "../utilities/sequence.h"

#include <iostream>

// the result of a test
enum Test_result
{
    PASSED = 0, // The test passed
    FAILED = 1, // The test failed
    IGNORE = 2, // The test should not be counted for some reason (for example if the test is not fully implemented yet)
};

// test macros to use in user-implemented tests
// Note that the boolean verbose must be defined somewhere accessible to your test
#define TEST(cond) do { if (!(cond)) { \
    if (verbose) std::cout << __FILE__ << ":" << __LINE__ << ": " << "Test failed: " << #cond << " is false" << std::endl; \
    return FAILED; \
}} while(0)

#define TEST_EQ(cond, value) do { if ((cond) != (value)) { \
    if (verbose) std::cout << __FILE__ << ":" << __LINE__ << ": " << "Test failed: " << #cond << " is " << cond << ", but should be " << value << std::endl; \
    return FAILED; \
}} while(0)

#define TEST_NOT_NULL(cond) do { if ((cond) == (nullptr)) { \
    if (verbose) std::cout << __FILE__ << ":" << __LINE__ << ": " << "Test failed: " << #cond << " is null" << std::endl; \
    return FAILED; \
}} while(0)

#define TEST_EXCEPT(statement) do { \
    try { statement; } catch(...) { break; } \
    if (verbose) std::cout << __FILE__ << ":" << __LINE__ << ": " << "Test failed: " << #statement << "; didn't throw exception" << std::endl; \
    return FAILED; \
} while(0)

#define TEST_NOEXCEPT(statement) do { \
    try { statement; } catch(...) { \
        if (verbose) std::cout << __FILE__ << ":" << __LINE__ << ": " << "Test failed: " << #statement << "; threw exception" << std::endl; \
        return FAILED; \
}} while(0)

// function signature of function tests
typedef Test_result (*test_fn)(void);

// run tests in a test suite and return the result
// if print_result is true, also print a test summary to std::cout
Test_result run_suite(const Seq<test_fn>& suite, const std::string& suite_name="", bool print_result=true);

// run all pre-defined suites defined below
Test_result run_default_test_suites();
Test_result run_default_test_suites_verbose(); // force verbose output
Test_result run_default_test_suites_quiet(); // force quiet output

// Pre-defined suites
Test_result test_utilities();
Test_result test_utilities_verbose();
Test_result test_utilities_quiet();

Test_result test_types();
Test_result test_types_verbose();
Test_result test_types_quiet();
