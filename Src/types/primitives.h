#pragma once

#include "type.h"
#include <limits>

struct CB_Bool {
    static CB_Type type;
    bool v = false;
    CB_Bool() {}
    CB_Bool(bool b) { this->v = b; }
    operator bool() const { return v; } // necessary for compact if() statements
    std::string toS() const { return v? "true" : "false"; }
};



#define GENERATE_PRIMITIVE_OPERATORS(CB_t)                    \
bool operator==(const CB_t& o) const { return v == o.v; }     \
bool operator!=(const CB_t& o) const { return !(*this == o); }\
bool operator<(const CB_t& o) const { return v < o.v; }       \
bool operator>=(const CB_t& o) const { return !(*this < o); } \
bool operator>(const CB_t& o) const { return o < *this; }     \
bool operator<=(const CB_t& o) const { return !(o < *this); } \
CB_t operator+(const CB_t& o) const { return CB_t(v + o.v); } \
CB_t operator-(const CB_t& o) const { return CB_t(v - o.v); } \
CB_t operator*(const CB_t& o) const { return CB_t(v * o.v); } \
CB_t operator/(const CB_t& o) const { if (o == 0) return CB_t::MAX_VALUE;  return CB_t(v / o.v); } \
CB_t& operator++() { ++v; return *this; }                       \
CB_t& operator--() { --v; return *this; }                       \



#define CB_NUMBER_TYPE(T, CPP_t)                             \
struct CB_##T {                                              \
    static CB_Type type;                                     \
    static const CB_##T MIN_VALUE;                           \
    static const CB_##T MAX_VALUE;                           \
    CPP_t v = 0; /* default value */                         \
    CB_##T() {}                                              \
    CB_##T(const CPP_t& v) { this->v = v; }                  \
    std::string toS() const { return std::to_string(v); }    \
    GENERATE_PRIMITIVE_OPERATORS(CB_##T);                    \
};                                                           \


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
    static const CB_Int MIN_VALUE;
    static const CB_Int MAX_VALUE;
    int64_t v = 0;
    CB_Int() {}
    CB_Int(const int64_t& v) { this->v = v; }
    std::string toS() const { return std::to_string(v); }
    operator CB_i8() const { CB_i8 i; i.v = v; return i; }
    operator CB_i16() const { CB_i16 i; i.v = v; return i; }
    operator CB_i32() const { CB_i32 i; i.v = v; return i; }
    operator CB_i64() const { CB_i64 i; i.v = v; return i; }

    operator CB_u8() const { CB_u8 i; i.v = v; return i; }
    operator CB_u16() const { CB_u16 i; i.v = v; return i; }
    operator CB_u32() const { CB_u32 i; i.v = v; return i; }
    operator CB_u64() const { CB_u64 i; i.v = v; return i; }
    GENERATE_PRIMITIVE_OPERATORS(CB_Int);
};

// generic unsigned int - stored as u64 but can be implicitly casted to any integer type
struct CB_Uint {
    static CB_Type type;
    static const CB_Uint MIN_VALUE;
    static const CB_Uint MAX_VALUE;
    uint64_t v = 0;
    CB_Uint() {}
    CB_Uint(const uint64_t& v) { this->v = v; }
    std::string toS() const { return std::to_string(v); }
    operator CB_i8() const { CB_i8 i; i.v = v; return i; }
    operator CB_i16() const { CB_i16 i; i.v = v; return i; }
    operator CB_i32() const { CB_i32 i; i.v = v; return i; }
    operator CB_i64() const { CB_i64 i; i.v = v; return i; }

    operator CB_u8() const { CB_u8 i; i.v = v; return i; }
    operator CB_u16() const { CB_u16 i; i.v = v; return i; }
    operator CB_u32() const { CB_u32 i; i.v = v; return i; }
    operator CB_u64() const { CB_u64 i; i.v = v; return i; }
    GENERATE_PRIMITIVE_OPERATORS(CB_Uint);
};

// generic float - stored as f64 but can be implicitly casted to any floating point type
struct CB_Float {
    static CB_Type type;
    static const CB_Float MIN_VALUE;
    static const CB_Float MAX_VALUE;
    double v = 0.0;
    CB_Float() {}
    CB_Float(const double& v) { this->v = v; }
    std::string toS() const { return std::to_string(v); }
    operator CB_f32() const { CB_f32 i; i.v = v; return i; }
    operator CB_f64() const { CB_f64 i; i.v = v; return i; }
    GENERATE_PRIMITIVE_OPERATORS(CB_Float);
};



