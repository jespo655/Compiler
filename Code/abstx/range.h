#pragma once

#include "type.h"
#include <string>

/*
A range consists of two doubles, a start and an end.
Ranges can be used in for loops:

range r = 2.5 .. 5.2;
for (i in r) { ... }

is equivalent to c++:

r_start = 2.5;
r_end = 5.2;
step = 1; // the default step is 1
for (double i = r_start; i <= r_end; i += step) { ... }
*/
struct Type_range : Type
{
    std::string toS() const override { return "range"; }

    std::string toS(void const * value_ptr, int size=0) const override {
        ASSERT(size == 0 || size == byte_size())
        double const * vp  = (double const *) value_ptr;
        return std::to_string(vp[0]) + ".." + std::to_string(vp[1]);
    }

    int byte_size() const override { return 2 * sizeof(double); }
};

