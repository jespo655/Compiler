#pragma once
#include "type.h"
#include "primitives.h"
#include <string>


struct CB_Flag : CB_Type
{
    static CB_Type type;
    static const bool primitive = true;
    uint8_t v;
    CB_Flag() {}
    CB_Flag(const uint8_t& v) { this->v = v; }
    explicit operator CB_Uint() const { return 1ULL<<(v-1); }
    std::string toS() const override { return "flag("+std::to_string(v)+")"; }

    // code generation functions
    virtual ostream& generate_typedef(ostream& os) const override {
        os << "typedef uint8_t ";
        return generate_type(os);
    }
    virtual ostream& generate_literal(ostream& os, void const* raw_data) const override {
        ASSERT(raw_data);
        return os << *(uint8_t*)raw_data;
    }

};


#define GENERATE_FLAG_OPERATORS(CB_t)                                \
static CB_t operator+(const CB_t& i, const CB_Flag& f) {             \
    return i.v | ((CB_Uint)f).v; /* bitwise or */                    \
}                                                                    \
static CB_t operator-(const CB_t& i, const CB_Flag& f) {             \
    return i.v & ~((CB_Uint)f).v; /* bitwise negated and */          \
}                                                                    \
static CB_t operator+=(CB_t& i, const CB_Flag& f) { i = i + f; }     \
static CB_t operator-=(CB_t& i, const CB_Flag& f) { i = i - f; }     \
static bool operator==(const CB_t& i, const CB_Flag& f) { return i == i + f; } \
static bool operator!=(const CB_t& i, const CB_Flag& f) { return i == i - f; } \

GENERATE_FLAG_OPERATORS(CB_Uint);
GENERATE_FLAG_OPERATORS(CB_u8);
GENERATE_FLAG_OPERATORS(CB_u16);
GENERATE_FLAG_OPERATORS(CB_u32);
GENERATE_FLAG_OPERATORS(CB_u64);

static CB_Uint operator+(const CB_Flag& lhs, const CB_Flag& rhs) {
    CB_Uint flags = 0;
    return flags + lhs + rhs;
}


