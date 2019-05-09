
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

    return PASSED;
}

static Test_result range_test()
{
    return IGNORE;
}

static Test_result seq_test()
{
    return IGNORE;
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



