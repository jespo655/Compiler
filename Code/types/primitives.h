#pragma once

#include "type.h"
#include <limits>

struct CB_Bool {
    static CB_Type type;
    bool v = false;
    CB_Bool() {}
    CB_Bool(bool b) { this->v = b; }
    operator bool() { return v; } // necessary for compact if() statements
    std::string toS() const { return v? "true" : "false"; }
};
CB_Type CB_Bool::type = CB_Type("bool");



#define GENERATE_PRIMITIVE_OPERATORS(CB_t)                              \
bool operator==(const CB_t& a, const CB_t& b) { return a.v == b.v; }    \
bool operator!=(const CB_t& a, const CB_t& b) { return !(a == b); }     \
bool operator<(const CB_t& a, const CB_t& b) { return a.v < b.v; }      \
bool operator>=(const CB_t& a, const CB_t& b) { return !(a < b); }      \
bool operator>(const CB_t& a, const CB_t& b) { return b < a; }          \
bool operator<=(const CB_t& a, const CB_t& b) { return !(b < a); }      \
CB_t operator+(const CB_t& a, const CB_t& b) { return CB_t(a.v + b.v); }\
CB_t operator-(const CB_t& a, const CB_t& b) { return CB_t(a.v - b.v); }\
CB_t operator*(const CB_t& a, const CB_t& b) { return CB_t(a.v * b.v); }\
CB_t operator/(const CB_t& a, const CB_t& b) { if (b == 0) return CB_t::MAX_VALUE;  return CB_t(a.v / b.v); } \
CB_t& operator++(CB_t& a) { ++a.v; return a; }               \
CB_t& operator--(CB_t& a) { --a.v; return a; }               \



#define CB_NUMBER_TYPE(T, CPP_t)                             \
struct CB_##T {                                              \
    static CB_Type type;                                     \
    static CB_##T MIN_VALUE;                                 \
    static CB_##T MAX_VALUE;                                 \
    CPP_t v = 0;                                             \
    CB_##T() {}                                              \
    CB_##T(const CPP_t& v) { this->v = v; }                  \
    std::string toS() const { return std::to_string(v); }    \
};                                                           \
CB_Type CB_##T::type = CB_Type(#T);                          \
CB_##T CB_##T::MIN_VALUE = std::numeric_limits<CPP_t>::min();\
CB_##T CB_##T::MAX_VALUE = std::numeric_limits<CPP_t>::max();\
GENERATE_PRIMITIVE_OPERATORS(CB_##T);                                  \



CB_NUMBER_TYPE(i8, int8_t);
CB_NUMBER_TYPE(i16, int16_t);
CB_NUMBER_TYPE(i32, int32_t);
CB_NUMBER_TYPE(i64, int64_t);

CB_NUMBER_TYPE(u8, uint8_t);
CB_NUMBER_TYPE(u16, uint16_t);
CB_NUMBER_TYPE(u32, uint32_t);
CB_NUMBER_TYPE(u64, uint64_t);

CB_NUMBER_TYPE(f32, float);
CB_NUMBER_TYPE(f64, double);

// generic int - stored as i64 but can be implicitly casted to any integer type
struct CB_Int {
    static CB_Type type;
    static CB_Int MIN_VALUE;
    static CB_Int MAX_VALUE;
    int64_t v;
    CB_Int() {}
    CB_Int(const int64_t& v) { this->v = v; }
    std::string toS() const { return std::to_string(v); }
    operator CB_i8() { CB_i8 i; i.v = v; return i; }
    operator CB_i16() { CB_i16 i; i.v = v; return i; }
    operator CB_i32() { CB_i32 i; i.v = v; return i; }
    operator CB_i64() { CB_i64 i; i.v = v; return i; }

    operator CB_u8() { CB_u8 i; i.v = v; return i; }
    operator CB_u16() { CB_u16 i; i.v = v; return i; }
    operator CB_u32() { CB_u32 i; i.v = v; return i; }
    operator CB_u64() { CB_u64 i; i.v = v; return i; }
};
CB_Type CB_Int::type = CB_Type("int");
CB_Int CB_Int::MIN_VALUE = std::numeric_limits<int64_t>::min();
CB_Int CB_Int::MAX_VALUE = std::numeric_limits<int64_t>::max();
GENERATE_PRIMITIVE_OPERATORS(CB_Int);

// generic unsigned int - stored as u64 but can be implicitly casted to any integer type
struct CB_Uint {
    static CB_Type type;
    static CB_Uint MIN_VALUE;
    static CB_Uint MAX_VALUE;
    uint64_t v;
    CB_Uint() {}
    CB_Uint(const uint64_t& v) { this->v = v; }
    std::string toS() const { return std::to_string(v); }
    operator CB_i8() { CB_i8 i; i.v = v; return i; }
    operator CB_i16() { CB_i16 i; i.v = v; return i; }
    operator CB_i32() { CB_i32 i; i.v = v; return i; }
    operator CB_i64() { CB_i64 i; i.v = v; return i; }

    operator CB_u8() { CB_u8 i; i.v = v; return i; }
    operator CB_u16() { CB_u16 i; i.v = v; return i; }
    operator CB_u32() { CB_u32 i; i.v = v; return i; }
    operator CB_u64() { CB_u64 i; i.v = v; return i; }
};
CB_Type CB_Uint::type = CB_Type("uint");
CB_Uint CB_Uint::MIN_VALUE = std::numeric_limits<uint64_t>::min();
CB_Uint CB_Uint::MAX_VALUE = std::numeric_limits<uint64_t>::max();
GENERATE_PRIMITIVE_OPERATORS(CB_Uint);

// generic float - stored as f64 but can be implicitly casted to any floating point type
struct CB_Float {
    static CB_Type type;
    static CB_Float MIN_VALUE;
    static CB_Float MAX_VALUE;
    double v;
    CB_Float() {}
    CB_Float(const double& v) { this->v = v; }
    std::string toS() const { return std::to_string(v); }
    operator CB_f32() { CB_f32 i; i.v = v; return i; }
    operator CB_f64() { CB_f64 i; i.v = v; return i; }
};
CB_Type CB_Float::type = CB_Type("float");
CB_Float CB_Float::MIN_VALUE = std::numeric_limits<double>::min();
CB_Float CB_Float::MAX_VALUE = std::numeric_limits<double>::max();
GENERATE_PRIMITIVE_OPERATORS(CB_Float);



