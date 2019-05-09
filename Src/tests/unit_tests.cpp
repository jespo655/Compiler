#include "unit_tests.h"
#include "aware.h"

#include <sstream>

// verbose flag to control all the tests in this file
static bool verbose = true;

// Test the test functions themselves - this should always be run in the beginning of a suite
static Test_result test_test()
{
    TEST(true);
    TEST_EQ(1, 1);
    TEST_NOT_NULL((void*)0x432104);
    TEST_EXCEPT(throw 2);
    TEST_NOEXCEPT();
    return PASSED;
}

static Test_result aware_test()
{
    Aware::reset();
    TEST_EQ(Aware::created, 0);
    Aware v = Aware(1,2,3);
    TEST_EQ(Aware::created, 1);
    TEST_EQ(Aware::alive, 1);
    TEST_EQ(Aware::copied, 0);
    TEST_EQ(Aware::moved, 0);
    TEST_EQ(v.deleted, 0);
    TEST_EQ(v.moved_from, 0);
    TEST_EQ(v.moved_to, 0);
    TEST_EQ(v.copied_to, 0);

    Aware v2 = Aware(std::move(v));
    TEST_EQ(Aware::created, 2);
    TEST_EQ(Aware::alive, 1);
    TEST_EQ(Aware::copied, 0);
    TEST_EQ(Aware::moved, 1);
    TEST_EQ(v.deleted, 0);
    TEST_EQ(v.moved_from, 1);
    TEST_EQ(v.moved_to, 0);
    TEST_EQ(v.copied_to, 0);

    TEST_EQ(v2.deleted, 0);
    TEST_EQ(v2.moved_from, 0);
    TEST_EQ(v2.moved_to, 1);
    TEST_EQ(v2.copied_to, 0);

    Aware v3 = Aware(v2);
    Aware v4 = Aware(v2);
    v3 = v4;
    v3 = std::move(v4);
    TEST_EQ(v2.deleted, 0);
    TEST_EQ(v2.moved_from, 0);
    TEST_EQ(v2.moved_to, 1);
    TEST_EQ(v2.copied_to, 0);

    TEST_EQ(v3.deleted, 0);
    TEST_EQ(v3.moved_from, 0);
    TEST_EQ(v3.moved_to, 1);
    TEST_EQ(v3.copied_to, 2);

    TEST_EQ(v4.deleted, 0);
    TEST_EQ(v4.moved_from, 1);
    TEST_EQ(v4.moved_to, 0);
    TEST_EQ(v4.copied_to, 1);

    TEST(v.i != v2.i);
    TEST(v.i != v3.i);
    TEST(v.i != v4.i);
    TEST(v2.i != v3.i);
    TEST(v2.i != v4.i);
    TEST(v3.i != v4.i);

    TEST(v < v2 && v2 < v3 && v3 < v4);
    TEST(v <= v2 && v2 <= v3 && v3 <= v4 && v <= v);
    TEST(v4 > v3 && v3 > v2 && v2 > v);
    TEST(v4 >= v3 && v3 >= v2 && v2 >= v && v >= v);
    TEST(v == v && v != v2);

    std::stringstream ss;
    ss << v;
    Aware::reset();
    return PASSED;
}

Test_result run_suite(const Seq<test_fn>& suite, const std::string& suite_name, bool print_result)
{
    uint32_t passed = 0;
    uint32_t failed = 0;
    uint32_t ignored = 0;

    for (const test_fn& fn : suite) {
        auto result = fn();
        if (result == PASSED) {
            passed++;
        } else if (result == FAILED) {
            failed++;
        } else {
            ignored++;
        }
    }

    if (print_result) {
        std::cout << "Test suite ";
        if (suite_name != "") std::cout << "\"" << suite_name << "\" ";
        std::cout << "complete: " << passed << "/" << (passed + failed) << " tests passed";
        if (ignored > 0) std::cout << " (" << ignored << " ignored)";
        std::cout << std::endl;
    }
    return (failed==0) ? PASSED : FAILED;
}

Test_result run_default_test_suites()
{
    Seq<test_fn> suite = {
        test_test,
        aware_test,
        (verbose ? test_utilities_verbose : test_utilities_quiet),
        (verbose ? test_types_verbose : test_types_quiet),
    };

    return run_suite(suite, "default suites", true);
}

Test_result run_default_test_suites_verbose()
{
    verbose = true;
    return run_default_test_suites();
}

Test_result run_default_test_suites_quiet()
{
    verbose = false;
    return run_default_test_suites();
}
