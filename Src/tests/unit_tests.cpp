#include "unit_tests.h"
#include "../utilities/flag.h"
#include "../utilities/sequence.h"

#include <iostream>
#include <exception>


static bool verbose = true;


#define TEST(cond) do { if (!(cond)) { \
    if (verbose) std::cout << "Test failed: " << #cond << " is false" << std::endl; \
    return FAILED; \
}} while(0)

#define TEST_EQ(cond, value) do { if ((cond) != (value)) { \
    if (verbose) std::cout << "Test failed: " << #cond << " is " << cond << ", but should be " << #value << std::endl; \
    return FAILED; \
}} while(0)

#define TEST_NOT_NULL(cond) do { if ((cond) == (nullptr)) { \
    if (verbose) std::cout << "Test failed: " << #cond << " is null" << std::endl; \
    return FAILED; \
}} while(0)

#define TEST_EXCEPT(statement) do { \
    try { statement; } catch(...) { break; } \
    if (verbose) std::cout << "Test failed: " << #statement << "; didn't throw exception" << std::endl; \
    return FAILED; \
} while(0)

#define TEST_NOEXCEPT(statement) do { \
    try { statement; } catch(...) { \
        if (verbose) std::cout << "Test failed: " << #statement << "; threw exception" << std::endl; \
        return FAILED; \
}} while(0)



static Test_result test_test()
{
    TEST(true);
    TEST_EQ(1, 1);
    TEST_NOT_NULL((void*)0x432104);
    TEST_EXCEPT(throw 2);
    TEST_NOEXCEPT();
    return PASSED;
}

static Test_result flag_test()
{
    flag f0 = 0;
    flag f1 = 1;
    flag f2 = 2;
    flag f3 = 3;
    flag f4 = 4;
    flag f5 = 5;
    flag f8 = 8;
    flag f10 = 10;
    flag f16 = 16;
    flag f63 = 63;
    flag f64 = 64;
    flag f65 = 65;

    TEST_EQ((uint8_t)f0, 0);
    TEST_EQ((uint32_t)f0+f2, 0x02);
    TEST_EQ(f1+f2, 0x03);
    TEST_EQ(f1+f8, 0x81);
    TEST_EQ(f8+f1, 0x81);
    TEST_EQ((uint64_t)f63, 0x4000000000000000);
    TEST_EQ((uint64_t)f64, 0x8000000000000000);
    return PASSED;
}


static Test_result seq_test()
{
    return PASSED;
}


static Test_result pointer_test()
{
    return PASSED;
}














Test_result test_types()
{
    Seq<test_fn> suite = {
        test_test,
        flag_test,
        // seq_test,
        // pointer_test,
    };

    uint32_t passed = 0;
    uint32_t failed = 0;

    for (test_fn fn : suite) {
        if (fn() == PASSED) {
            passed++;
        } else {
            failed++;
        }
    }

    std::cout << "Test suite complete: " << passed << "/" << suite.size << " tests passed" << std::endl;
    return (failed==0) ? PASSED : FAILED;
}

