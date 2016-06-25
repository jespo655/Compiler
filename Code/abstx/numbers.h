#pragma once

#include "type.h"
#include <string>


#define NUMBER_TYPE(type, cpptype)                                          \
struct Type_##type : Type                                                   \
{                                                                           \
    std::string toS() const override { return "##type##"; }                 \
    int byte_size() const override { return sizeof(cpptype); }              \
                                                                            \
    std::string toS(void const * value_ptr, int size=0) const override {    \
        ASSERT(size == 0 || size == byte_size())                            \
        cpptype const * vp  = (cpptype const *) value_ptr;                  \
        return std::to_string(vp[0]);                                       \
    }                                                                       \
};

NUMBER_TYPE(i8, int_least8_t);
NUMBER_TYPE(i16, int_least16_t);
NUMBER_TYPE(i32, int_least32_t);
NUMBER_TYPE(i64, int_least64_t);

NUMBER_TYPE(u8, uint_least8_t);
NUMBER_TYPE(u16, uint_least16_t);
NUMBER_TYPE(u32, uint_least32_t);
NUMBER_TYPE(u64, uint_least64_t);

NUMBER_TYPE(f32, float);
NUMBER_TYPE(f64, double);

#define Type_int Type_i64
#define Type_uint Type_u64




