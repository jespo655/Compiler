
#include "unit_tests.h"
#include "aware.h"

#include "../types/cb_any.h"
#include "../types/cb_function.h"
#include "../types/cb_pointer.h"
#include "../types/cb_primitives.h"
#include "../types/cb_range.h"
#include "../types/cb_seq.h"
#include "../types/cb_string.h"
#include "../types/cb_struct.h"
#include "../types/cb_type.h"

#include <sstream>
#include <ctime>

static bool verbose = true;

#define test_default_value(Type, size, default_literal) do { \
    Type t; \
    TEST_EQ(t.cb_sizeof(), size); \
    TEST_EQ(t.default_value(), 0); \
    std::stringstream ss{}; \
    t.generate_literal(ss, t.default_value().v_ptr); \
    TEST_EQ(ss.str(), default_literal); \
} while(0)

#define test_literal(Type, value, literal) do { \
    Type t; \
    Type::c_typedef v = value; \
    std::stringstream ss{}; \
    t.generate_literal(ss, &v); \
    TEST_EQ(ss.str(), literal); \
} while(0)

#define P99_PROTECT(...) __VA_ARGS__


static Test_result any_test()
{
    return IGNORE;
}

static Test_result function_test()
{
    return IGNORE;
}

static Test_result pointer_test()
{
    test_default_value(CB_Pointer, 8, "NULL");
    test_literal(CB_Pointer, (void*)0xDEADBEEF, "0xdeadbeef");

    CB_Pointer t;
    CB_Pointer t2;
    TEST(t.uid == t2.uid);
    std::stringstream ss{};
    ss << t.toS();
    TEST_EQ(ss.str(), "_cb_unresolved_pointer");

    t.v_type = CB_Int::type;
    t.finalize();
    TEST(t.uid != t2.uid);
    TEST(t.is_primitive());

    t2.v_type = CB_Int::type;
    t2.finalize();
    TEST(t.uid == t2.uid);

    CB_Pointer t3{CB_Uint::type, true};
    CB_Pointer t4{CB_Uint::type, true};
    TEST(t3.uid != t.uid);
    TEST(t3.uid == t4.uid);
    t4.owning = false;
    t4.finalize();
    TEST(t3.uid != t4.uid);

    return PASSED;
}

static Test_result primitives_test()
{
    test_default_value(CB_i8, 1, "0");
    test_default_value(CB_i16, 2, "0");
    test_default_value(CB_i32, 4, "0L");
    test_default_value(CB_i64, 8, "0LL");
    test_default_value(CB_Int, 8, "0LL");
    test_default_value(CB_u8, 1, "0");
    test_default_value(CB_u16, 2, "0");
    test_default_value(CB_u32, 4, "0UL");
    test_default_value(CB_u64, 8, "0ULL");
    test_default_value(CB_Uint, 8, "0ULL");
    test_default_value(CB_f32, 4, "0");
    test_default_value(CB_f64, 8, "0");
    test_default_value(CB_Float, 8, "0");
    test_default_value(CB_Flag, 1, "0");

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Woverflow"
    test_literal(CB_i8, -128, "-128");
    test_literal(CB_i8, -129, "127");
    test_literal(CB_i64, 0x8000000000000000, "-9223372036854775808LL");
    test_literal(CB_i64, 0x7FFFFFFFFFFFFFFF, "9223372036854775807LL");
    test_literal(CB_u8, 1, "1");
    test_literal(CB_u8, -1, "255");
    test_literal(CB_u16, -1, "65535");
    test_literal(CB_u32, -1, "4294967295UL");
    test_literal(CB_u64, -1, "18446744073709551615ULL");
    #pragma GCC diagnostic pop

    {
        CB_u8 t;
        TEST(t.is_primitive());
        std::stringstream ss{};
        t.generate_type(ss);
        ss << "; ";
        t.generate_typedef(ss);
        TEST_EQ(ss.str(), "_cb_u8; typedef uint8_t _cb_u8;\n");
    }

    {
        CB_Bool t;
        TEST(t.is_primitive());
        TEST_EQ(t.type, CB_Bool::type);
        CB_Bool::c_typedef F = false;
        CB_Bool::c_typedef T = true;
        std::stringstream ss;
        t.generate_literal(ss, &F);
        t.generate_literal(ss, &T);
        TEST_EQ(ss.str(), "01");
        TEST_EQ(t.cb_sizeof(), 1);
        TEST_EQ(t.default_value(), false);
        TEST(t.default_value() != true);
    }

    CB_Int i1;
    CB_Int i2;
    CB_Uint u1;
    CB_Uint u2;
    TEST(i1.uid == i2.uid);
    TEST(u1.uid == u2.uid);
    TEST(i1.uid != u1.uid);

    return PASSED;
}

static Test_result range_test()
{
    test_default_value(CB_Range, 16, "(_cb_i_range){0LL, 0LL}");
    test_default_value(CB_Float_range, 16, "(_cb_f_range){0, 0}");
    test_literal(CB_Range, P99_PROTECT({-5, 2}), "(_cb_i_range){-5LL, 2LL}");
    test_literal(CB_Float_range, P99_PROTECT({-5.2, 2.5}), "(_cb_f_range){-5.2, 2.5}");
    CB_Range r1;
    CB_Range r2;
    CB_Float_range f1;
    CB_Float_range f2;
    TEST(r1.uid == r2.uid);
    TEST(f1.uid == f2.uid);
    TEST(r1.uid != f1.uid);
    std::stringstream ss;
    r1.generate_for(ss, "r", "i");
    TEST_EQ(ss.str(), "for (_cb_i64 i = r.r_start; i <= r.r_end; i += 1)");
    ss.str(std::string());
    ss.clear();
    f1.generate_for(ss, "r", "i");
    TEST_EQ(ss.str(), "for (_cb_f64 i = r.r_start; i <= r.r_end; i += 1)");
    // TODO: compile and run a range snippet
    ss.str(std::string());
    ss.clear();
    TEST(!r1.is_primitive());
    return PASSED;
}

static Test_result seq_test()
{
    CB_Seq s1;
    CB_Seq s2;
    CB_Fixed_seq f1;
    CB_Fixed_seq f2;
    // std::string _cb_unresolved_seq_name = toS()
    // std::string _cb_unresolved_fixed_seq_name =
    test_default_value(CB_Seq, 16, "(_cb_type_21){0UL, 0UL, NULL}");
    // test_default_value(CB_Fixed_seq, 8, "NULL"); // @TODO: check this
    test_literal(CB_Seq, P99_PROTECT({5, 2, (void*)0x123}), "(_cb_type_21){5UL, 2UL, 0x123}");
    // test_literal(CB_Fixed_seq, P99_PROTECT((void*)0x123), "0x123"); // @TODO: check this
    TEST(s1.uid == s2.uid);
    TEST(f1.uid == f2.uid);
    // TEST(s1.uid != f1.uid); // @TODO: check this
    TEST(!s1.is_primitive());
    TEST(!f1.is_primitive());
    std::stringstream ss{};
    ss << s1.toS() << "; " << f1.toS();;
    TEST_EQ(ss.str(), "_cb_unresolved_seq; _cb_unresolved_fixed_seq");
    ss.str(std::string());
    ss.clear();

    s1.v_type = CB_Int::type;
    s1.finalize();
    TEST(s1.uid != s2.uid);
    TEST(!s1.is_primitive());
    s2.v_type = CB_Int::type;
    s2.finalize();
    TEST(s1.uid == s2.uid);

    s1.generate_for(ss, "r", "i");
    TEST_EQ(ss.str(), "for (_cb_i64 i = r.r_start; i <= r.r_end; i += 1)");
    TEST(false);
    ss.str(std::string());
    ss.clear();
    // f1.generate_for(ss, "r", "i");
    // TEST_EQ(ss.str(), "for (_cb_f64 i = r.r_start; i <= r.r_end; i += 1)");
    // // TODO: compile and run a range snippet
    // ss.str(std::string());
    // ss.clear();


    return PASSED;
}

static Test_result string_test()
{
    return IGNORE;
}

static Test_result struct_test()
{
    return IGNORE;
}

static Test_result type_test()
{
    return IGNORE;
}


Test_result test_types()
{
    Seq<test_fn> suite = {
        any_test,
        function_test,
        pointer_test,
        primitives_test,
        range_test,
        seq_test,
        string_test,
        struct_test,
        type_test,
    };

    return run_suite(suite, "types");
}

Test_result test_types_verbose()
{
    verbose = true;
    return test_types();
}

Test_result test_types_quiet()
{
    verbose = false;
    return test_types();
}



