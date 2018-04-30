#pragma once

#include "type.h"

#define GENERATE_PRIMITIVE(cpp_type, tos, c_type) \
struct cpp_type : CB_Type { \
    static CB_Type type; \
    static const bool primitive = true; \
    cpp_type() { uid = type.uid; } \
    std::string toS() const override { return name; } \
\
    virtual ostream& generate_typedef(ostream& os) const override { \
        os << "typedef " << c_type << " "; \
        generate_type(os); \
        return os << ";"; \
    } \
    virtual ostream& generate_literal(ostream& os, void const* raw_data) const override { \
        ASSERT(raw_data); \
        return os << *(c_type*)raw_data; \
    } \
}

GENERATE_PRIMITIVE(CB_Bool, "bool", bool);

GENERATE_PRIMITIVE(CB_i8, "i8", int8_t);
GENERATE_PRIMITIVE(CB_i16, "i16", int16_t);
GENERATE_PRIMITIVE(CB_i32, "i32", int32_t);
GENERATE_PRIMITIVE(CB_i64, "i64", int64_t);

GENERATE_PRIMITIVE(CB_u8, "u8", uint8_t);
GENERATE_PRIMITIVE(CB_u16, "u16", uint16_t);
GENERATE_PRIMITIVE(CB_u32, "u32", uint32_t);
GENERATE_PRIMITIVE(CB_u64, "u64", uint64_t);

GENERATE_PRIMITIVE(CB_f32, "f32", float);
GENERATE_PRIMITIVE(CB_f64, "f64", double);

GENERATE_PRIMITIVE(CB_Flag, "flag", uint8_t);
