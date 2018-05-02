#pragma once

#include "cb_type.h"

#define GENERATE_PRIMITIVE(cpp_type, tos, c_type, literal_suffix) \
struct cpp_type : CB_Type { \
    static const shared<const CB_Type> type; \
    static constexpr c_type _default_value = 0; \
    cpp_type() { uid = type->uid; } \
    cpp_type(const std::string& name, size_t size, void const* default_value) : CB_Type(name, size, default_value) {} \
    std::string toS() const override { return tos; } \
    bool is_primitive() const override { return true; } \
    void generate_typedef(ostream& os) const override { \
        os << "typedef " << #c_type << " "; \
        generate_type(os); \
        os << ";"; \
    } \
    void generate_literal(ostream& os, void const* raw_data, uint32_t depth = 0) const override { \
        ASSERT(raw_data); \
        os << *(c_type*)raw_data << literal_suffix;\
    } \
}

GENERATE_PRIMITIVE(CB_Bool, "bool", bool, "");

GENERATE_PRIMITIVE(CB_i8, "i8", int8_t, "");
GENERATE_PRIMITIVE(CB_i16, "i16", int16_t, "");
GENERATE_PRIMITIVE(CB_i32, "i32", int32_t, "L");
GENERATE_PRIMITIVE(CB_i64, "i64", int64_t, "LL");
GENERATE_PRIMITIVE(CB_Int, "int", int64_t, "LL");

GENERATE_PRIMITIVE(CB_u8, "u8", uint8_t, "");
GENERATE_PRIMITIVE(CB_u16, "u16", uint16_t, "");
GENERATE_PRIMITIVE(CB_u32, "u32", uint32_t, "UL");
GENERATE_PRIMITIVE(CB_u64, "u64", uint64_t, "ULL");
GENERATE_PRIMITIVE(CB_Uint, "uint", uint64_t, "ULL");

GENERATE_PRIMITIVE(CB_f32, "f32", float, "");
GENERATE_PRIMITIVE(CB_f64, "f64", double, "");
GENERATE_PRIMITIVE(CB_Float, "float", double, "");

GENERATE_PRIMITIVE(CB_Flag, "flag", uint8_t, "");


