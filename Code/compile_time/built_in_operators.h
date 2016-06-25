#pragma once


#ifdef __cplusplus
extern "C" {
#endif

// #ifndef __cplusplus

    #ifndef int_least8_t
    #define int_least8_t signed char
    #endif

    #ifndef int_least16_t
    #define int_least16_t signed short
    #endif

    #ifndef int_least32_t
    #define int_least32_t signed long
    #endif

    #ifndef int_least64_t
    #define int_least64_t signed long long
    #endif

    #ifndef uint_least8_t
    #define uint_least8_t unsigned char
    #endif

    #ifndef uint_least16_t
    #define uint_least16_t unsigned short
    #endif

    #ifndef uint_least32_t
    #define uint_least32_t unsigned long
    #endif

    #ifndef uint_least64_t
    #define uint_least64_t unsigned long long
    #endif

// #endif


    #define INFIX_OPERATOR(name, op, lhs_type, rhs_type, rval_type)     \
    void _infix_operator_##name(lhs_type lhs, rhs_type rhs, rval_type* __rval_1) { *__rval_1 = lhs op rhs; }


    #define INFIX_GENERATOR_INT(name, op)                                               \
    /* int + int */                                                                     \
    INFIX_OPERATOR(name##_i8_i8, op, int_least8_t, int_least8_t, int_least8_t);         \
    INFIX_OPERATOR(name##_i16_i16, op, int_least16_t, int_least16_t, int_least16_t);    \
    INFIX_OPERATOR(name##_i32_i32, op, int_least32_t, int_least32_t, int_least32_t);    \
    INFIX_OPERATOR(name##_i64_i64, op, int_least64_t, int_least64_t, int_least64_t);    \
    /* uint + uint */                                                                   \
    INFIX_OPERATOR(name##_u8_u8, op, uint_least8_t, uint_least8_t, uint_least8_t);      \
    INFIX_OPERATOR(name##_u16_u16, op, uint_least16_t, uint_least16_t, uint_least16_t); \
    INFIX_OPERATOR(name##_u32_u32, op, uint_least32_t, uint_least32_t, uint_least32_t); \
    INFIX_OPERATOR(name##_u64_u64, op, uint_least64_t, uint_least64_t, uint_least64_t);

    #define INFIX_GENERATOR_FLOAT(name, op)                             \
    INFIX_OPERATOR(name##_float_float, op, float, float, float);        \
    INFIX_OPERATOR(name##_double_double, op, double, double, double);

    #define INFIX_GENERATOR(name, op) \
    INFIX_GENERATOR_FLOAT(name, op)   \
    INFIX_GENERATOR_INT(name, op)


    INFIX_GENERATOR(plus,+);
    INFIX_GENERATOR(minus,-);
    INFIX_GENERATOR(mult,*);
    INFIX_GENERATOR(div,/);

    INFIX_GENERATOR_INT(mod,%);





#ifdef __cplusplus
} // extern "C"
#endif



/*#ifdef __cplusplus
#include "../utilities/assert.h"
void assert_sizes() {
    ASSERT(sizeof(int_least8_t) >= 8);
    ASSERT(sizeof(int_least16_t) >= 16);
    ASSERT(sizeof(int_least32_t) >= 32);
    ASSERT(sizeof(int_least64_t) >= 64);

    ASSERT(sizeof(uint_least8_t) >= 8);
    ASSERT(sizeof(uint_least16_t) >= 16);
    ASSERT(sizeof(uint_least32_t) >= 32);
    ASSERT(sizeof(uint_least64_t) >= 64);

    ASSERT(sizeof(float) >= 32);
    ASSERT(sizeof(double) >= 64);
}
#endif*/