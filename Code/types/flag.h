#pragma once
#include "type.h"
#include "primitives.h"
#include <string>


struct CB_Flag
{
    static CB_Type type;
    uint8_t v;
    CB_Flag() {}
    CB_Flag(const uint8_t& v) { this->v = v; }
    std::string toS() const { return "flag("+std::to_string(v)+")"; }
};
CB_Type CB_Flag::type = CB_Type("flag");


#define GENERATE_FLAG_OPERATORS(CB_t)               \
CB_t operator+(const CB_t& i, const CB_Flag& f) {   \
    if (f.v > 8*sizeof(i)) return i;                \
    CB_t fv = 1<<f.v;                               \
    return i.v | fv.v; /* bitwise or */             \
}                                                   \
CB_t operator-(const CB_t& i, const CB_Flag& f) {   \
    if (f.v > 8*sizeof(i)) return i;                \
    CB_t fv = 1<<f.v;                               \
    return i.v & ~fv.v; /* bitwise negated and */   \
}                                                   \
CB_t operator==(const CB_t& i, const CB_Flag& f) { return i != (i - f); } \
CB_t operator!=(const CB_t& i, const CB_Flag& f) { return i != (i + f); } \



GENERATE_FLAG_OPERATORS(CB_Uint);
GENERATE_FLAG_OPERATORS(CB_u8);
GENERATE_FLAG_OPERATORS(CB_u16);
GENERATE_FLAG_OPERATORS(CB_u32);
GENERATE_FLAG_OPERATORS(CB_u64);


