#pragma once

#include <string>

// Below: utilities version of flag that can be used in c++

struct flag
{
    uint8_t v;
    flag(const uint8_t& v) { this->v = v; }
    explicit operator uint64_t() const { return 1ULL<<(v-1); }
};

#define GENERATE_FLAG_OPERATORS(T)                                   \
static T operator+(const T& i, const flag& f) {                      \
    return i | ((uint64_t)f); /* bitwise or */                       \
}                                                                    \
static T operator-(const T& i, const flag& f) {                      \
    return i & ~((uint64_t)f); /* bitwise negated and */             \
}                                                                    \
static T operator+=(T& i, const flag& f) { i = i + f; }              \
static T operator-=(T& i, const flag& f) { i = i - f; }              \
static bool operator==(const T& i, const flag& f) { return i == i + f; } \
static bool operator!=(const T& i, const flag& f) { return i == i - f; } \

GENERATE_FLAG_OPERATORS(uint8_t);
GENERATE_FLAG_OPERATORS(uint16_t);
GENERATE_FLAG_OPERATORS(uint32_t);
GENERATE_FLAG_OPERATORS(uint64_t);

static uint64_t operator+(const flag& lhs, const flag& rhs) {
    uint64_t flags = 0;
    return flags + lhs + rhs;
}


