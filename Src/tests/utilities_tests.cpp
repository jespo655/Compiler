
#include "unit_tests.h"
#include "aware.h"

#include "../utilities/assert.h"
#include "../utilities/debug.h"
#include "../utilities/error_handler.h"
#include "../utilities/flag.h"
#include "../utilities/pointers.h"
#include "../utilities/sequence.h"
#include "../utilities/static_any.h"
#include "../utilities/unique_id.h"
#include "../parser/token.h"

#include <sstream>
#include <ctime>

static bool verbose = true;

// assert
static Test_result assert_test()
{
    // we can't check for failing tests since that would exit the program
    ASSERT(true);
    ASSERT(true, "message");
    int complex=57, expression=2;
    ASSERT(complex*expression > complex+expression);
    return PASSED;
}

// debug
static Test_result debug_test()
{
    std::stringstream ss;
    Debug_os os(ss);
    os.indent();
    os.indent();
    os.unindent(2);
    os << "a" << std::endl;
    os.set_indent_level(1);
    os.unindent(2);
    os << "a" << std::endl;
    os.set_indent_level(1);
    os << "a" << std::endl;
    os.indent();
    os << "a" << std::endl;
    os.unindent();
    os << "a" << std::endl;
    TEST_EQ(ss.str(), "a\na\n    a\n        a\n    a\n");
    return PASSED;
}

// error handler
static Test_result error_handler_test()
{
    Token_context c{};
    c.line = __LINE__;
    c.position = 4;
    c.file = __FILE__;
    set_logging(false);
    log_error("", c);
    log_warning("", c);
    add_note("", c);
    add_note("");
    exit_if_errors(); // shouldn't exit since logging is off
    set_logging(true); // back to default
    return PASSED;
}

// flag
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

// pointers
static Owned<Aware> create_aware() { return alloc<Aware>(1,2,3); }
static void grab_aware(Owned<Aware> p) {}
static void share_aware(Shared<Aware> p) {}

static Test_result pointers_test()
{
    Aware::reset();
    TEST_EQ(Aware::created, 0);
    Owned<Aware> o = create_aware();
    TEST_EQ(Aware::created, 1);
    TEST_EQ(Aware::alive, 1);
    TEST_NOT_NULL(o);
    share_aware(o);
    TEST_EQ(Aware::created, 1);
    TEST_EQ(Aware::alive, 1);
    TEST_NOT_NULL(o);
    Shared<Aware> s = o;
    Shared<Aware> s2 = o;
    TEST_NOT_NULL(s);
    TEST(s == o && s == s2);
    Owned<Aware> o2 = std::move(o);
    TEST_EQ(o, nullptr);
    grab_aware(std::move(o2));
    TEST_EQ(o2, nullptr);
    TEST_EQ(Aware::created, 1);
    TEST_EQ(Aware::alive, 0);
    return PASSED;
}

// sequence

template<typename T> void grab_value(T t) {}
template<typename T> void grab_reference(const T& t) {}
template<typename T> void grab_move(T&& t) {}

static Test_result sequence_test()
{
    Aware::reset();
    Seq<Aware> seq;
    TEST_EQ(Aware::created, 0);
    seq.resize(4);
    TEST_EQ(Aware::created, 4);
    seq.resize(10);
    TEST_EQ(Aware::created, 14);
    TEST_EQ(Aware::alive, 10);
    TEST_EQ(Aware::moved, 4);
    TEST_EQ(seq[0].moved_to, 1);
    TEST_EQ(seq[3].moved_to, 1);
    TEST_EQ(seq[4].moved_to, 0);
    TEST_EQ(seq[9].moved_to, 0);
    for (const auto& a : seq) { TEST_EQ(a.copied_to, 0); }
    TEST_EQ(Aware::created, 14);
    TEST_EQ(Aware::alive, 10);
    TEST_EQ(Aware::moved, 4);
    TEST_EQ(seq.size, 10);
    seq.remove_last();
    TEST_EQ(seq.size, 9);
    seq.add(Aware()); // Note that this creates one extra object that gets immediately moved from
    TEST_EQ(seq.size, 10);
    TEST_EQ(Aware::created, 16);
    TEST_EQ(Aware::alive, 10);
    TEST_EQ(Aware::moved, 5);
    TEST_EQ(Aware::copied, 0);
    seq.clear();
    TEST_EQ(Aware::alive, 0);

    Aware::reset();
    TEST_EQ(Aware::created, 0);
    Fixed_seq<Aware, 4> fseq;
    TEST_EQ(Aware::created, 4);
    const Aware& a = fseq[2];
    fseq[3] = fseq[2];
    TEST_EQ(Aware::created, 4);
    TEST_EQ(Aware::copied, 1);
    grab_reference(fseq);
    TEST_EQ(Aware::created, 4);
    grab_value(fseq);
    TEST_EQ(Aware::created, 8);
    TEST_EQ(Aware::alive, 4);
    grab_move(std::move(fseq));
    TEST_EQ(Aware::created, 8);
    TEST_EQ(Aware::alive, 4);
    Fixed_seq<Aware, 4> fseq2;
    TEST_EQ(Aware::created, 12);

    return PASSED;
}

// any
static Test_result any_test()
{
    Aware::reset();
    Static_any any;
    TEST_EQ(any.has_type<void>(), true);
    TEST_EQ(any.has_value<void>(), false);
    TEST_EQ(any.has_type<Aware>(), false);
    any.allocate<Aware>(true);
    TEST(any.toS() == any.value<Aware>().toS());
    any.value<Aware>().i = 0;
    TEST_EQ(any.toS(), "Aware(0)");
    TEST_EQ(any.has_type<void>(), false);
    TEST_EQ(any.has_type<Aware>(), true);
    TEST_EQ(any.has_value<Aware>(), true);
    TEST_EQ(Aware::created, 1);
    TEST_EQ(Aware::alive, 1);
    Aware a;
    TEST_EQ(Aware::created, 2);
    TEST_EQ(Aware::alive, 2);
    any = a;
    TEST_EQ(Aware::created, 2);
    TEST_EQ(Aware::alive, 2);
    TEST_EQ(Aware::copied, 1);
    TEST_EQ(Aware::moved, 0);
    any = std::move(a);
    TEST_EQ(Aware::created, 2);
    TEST_EQ(Aware::alive, 1);
    TEST_EQ(Aware::copied, 1);
    TEST_EQ(Aware::moved, 1);
    int i = 3;
    any = i;
    TEST_EQ(Aware::alive, 0);
    TEST_EQ(any.has_value<int>(), true);
    TEST_EQ(any, 3);
    any.deallocate();
    TEST_EQ(any.has_type<void>(), true);
    return PASSED;
}

// unique ids
static Test_result uid_test()
{
    // Do as many tests as possible during a certain time
    std::clock_t start = std::clock();
    const int max_uids = 10000;
    const int max_ms = 100;
    uint64_t uids[max_uids];

    for (int i = 0; i < max_uids; ++i) {
        uids[i] = get_unique_id();
    }

    int loops = 0;
    uint64_t elapsed_ms = 0;
    while (elapsed_ms < max_ms) {
        for (int i = 0; i < max_uids; ++i) {
            for (int j = 0; j < max_uids; ++j) {
                TEST(i == j || uids[i] != uids[j]);
            }
            uids[i] = get_unique_id();
        }
        ++loops;
        elapsed_ms = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
    }

    TEST(loops >= 1);
    // std::cout << "Uid test finished " << loops << " loops in " << elapsed_ms << " ms" << std::endl;
    return PASSED;
}




Test_result test_utilities()
{
    Seq<test_fn> suite = {
        assert_test,
        debug_test,
        error_handler_test,
        pointers_test,
        sequence_test,
        any_test,
        uid_test,
    };

    return run_suite(suite, "utilities");
}

Test_result test_utilities_verbose()
{
    verbose = true;
    return test_utilities();
}

Test_result test_utilities_quiet()
{
    verbose = false;
    return test_utilities();
}
