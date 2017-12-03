#pragma once

#include "type.h"
#include "primitives.h"

#include <string>

/*
Range - a compact data structure with a start and an end.
Can be iterated over

Syntax:
a : Range = 0..2;
*/

struct CB_Range {
    static CB_Type type;
    CB_f64 r_start;
    CB_f64 r_end; // has to have r_ prefix to not collide with end() for range-based cpp loops. In CB, call this just "end"

    std::string toS() const {
        return r_start.toS() + ".." + r_end.toS();
    }

    struct iterator {
        CB_f64 current;
        CB_f64 step=1;
        const CB_Range& range;
        iterator(const CB_Range& range, CB_f64 current, CB_f64 step=1) : range{range}, current{current}, step{step} {}
        CB_f64 operator*() const { ASSERT(current <= range.r_end); return current; }
        iterator& operator++() { current = current + step; return *this; }
        bool operator==(const iterator& o) const { return current == o.current || current > range.r_end && o.current > range.r_end; }
        bool operator!=(const iterator& o) const { return !(*this == o); }
        bool operator<(const iterator& o) const { return current < o.current; }
    };

};
CB_Type CB_Range::type = CB_Type("range", CB_Range());


static CB_Range::iterator begin(const CB_Range& r) { return CB_Range::iterator(r, r.r_start); }
static CB_Range::iterator end(const CB_Range& r) { return CB_Range::iterator(r, r.r_end + 1); }
