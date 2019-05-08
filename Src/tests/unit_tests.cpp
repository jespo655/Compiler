#include "unit_tests.h"
#include "../utilities/assert.h"
#include "../utilities/debug.h"
#include "../utilities/error_handler.h"
#include "../utilities/flag.h"
#include "../utilities/pointers.h"
#include "../utilities/sequence.h"
#include "../utilities/static_any.h"
#include "../utilities/unique_id.h"
#include "../parser/token.h"

#include <iostream>
#include <sstream>

// verbose flag to control all the tests in this file
static bool verbose = true;

void set_verbose_default_tests(bool v)
{
    verbose = v;
}

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



// Class that's aware to copy/move constructions/assignments and deletions
struct Aware : public Serializable
{
    static int created; // incremented on any constructor
    static int alive; // incremented on any constructor; decremented on destructor or move from
    static int moved; // incremented on any move assignment or constructor
    static int copied; // incremented on any copy assignment or constructor

    int i = get_unique_id();
    int deleted = 0; // incremented on destructor. Can only be done once.
    // int copied_from = 0; // incremented on copy assignment from this. Illegal with const reference argument
    int moved_from = 0; // incremented on move assignment from this
    int copied_to = 0; // incremented on copy assignment to this
    int moved_to = 0; // incremented on move assignment to this

    Aware() { created++; alive++; }
    template<typename... Args>Aware(Args... args) { created++; alive++; }
    ~Aware() { ASSERT(!deleted); deleted++; if (!moved_from) alive--; }
    Aware(const Aware& o) { created++; alive++; *this = o; }
    Aware& operator=(const Aware& o) { ASSERT(!deleted); ASSERT(!o.deleted); ASSERT(!o.moved_from); copied++; copied_to++; return *this; }
    Aware(Aware&& o) { created++; alive++; *this = std::move(o); }
    Aware& operator=(Aware&& o) { ASSERT(!deleted); ASSERT(!o.deleted); ASSERT(!o.moved_from); moved++; moved_to++; o.moved_from++; alive--; return *this; }

    bool operator==(const Aware& o) const { return i == o.i; }
    bool operator!=(const Aware& o) const { return !(*this == o); }
    bool operator<(const Aware& o) const { return i < o.i; }
    bool operator>(const Aware& o) const { return o < *this; }
    bool operator>=(const Aware& o) const { return !(*this < o); }
    bool operator<=(const Aware& o) const { return !(o < *this); }

    std::string toS() const { return "Aware(" + std::to_string(i) + ")"; }

    static void reset() {
        Aware::created = 0;
        Aware::alive = 0;
        Aware::moved = 0;
        Aware::copied = 0;
    }
};

int Aware::created = 0;
int Aware::alive = 0;
int Aware::copied = 0;
int Aware::moved = 0;

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









// utilities tests

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
    std::cout << any << std::endl;
    // TEST_EQ(any.has_type<Aware>(), true);
    // TEST_EQ(any.has_value<Aware>(), true);
    // TEST_EQ(Aware::created, 1);
    // TEST_EQ(Aware::alive, 1);
    // Aware a;
    // TEST_EQ(Aware::created, 2);
    // TEST_EQ(Aware::alive, 2);
    // any = a;
    // TEST_EQ(Aware::created, 2);
    // TEST_EQ(Aware::alive, 2);
    // TEST_EQ(Aware::copied, 1);
    // TEST_EQ(Aware::moved, 0);
    // any = std::move(a);
    // TEST_EQ(Aware::created, 2);
    // TEST_EQ(Aware::alive, 1);
    // TEST_EQ(Aware::copied, 1);
    // TEST_EQ(Aware::moved, 1);
    // int i = 3;
    // any = i;
    // TEST_EQ(Aware::alive, 0);
    // TEST_EQ(any.has_value<int>(), true);
    // TEST_EQ(any, 3);
    std::cout << "end" << std::endl;
    return PASSED;
}

// unique ids
static Test_result uid_test()
{
    return PASSED;
}







Test_result run_suite(const Seq<test_fn>& suite, const std::string& suite_name, bool print_result)
{
    uint32_t passed = 0;
    uint32_t failed = 0;

    for (const test_fn& fn : suite) {
        if (fn() == PASSED) {
            passed++;
        } else {
            failed++;
        }
    }

    if (print_result) {
        std::cout << "Test suite ";
        if (suite_name != "") std::cout << "\"" << suite_name << "\" ";
        std::cout << "complete: " << passed << "/" << suite.size << " tests passed" << std::endl;
    }
    return (failed==0) ? PASSED : FAILED;
}










Test_result test_utilities()
{
    Seq<test_fn> suite = {
        test_test,
        aware_test,
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

Test_result test_types()
{
    Seq<test_fn> suite = {
        // test_test,
        // @TODO: extend with all cb types
    };

    return run_suite(suite, "types");
}

Test_result run_default_test_suites()
{
    Seq<test_fn> suite = {
        test_utilities,
        test_types,
    };

    return run_suite(suite, "default suites", false);
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
