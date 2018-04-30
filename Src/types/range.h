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

struct CB_Range : CB_Type {
    static CB_Type type;
    static const bool primitive = false;

    CB_Range() { uid = type.uid; }
    std::string toS() const override { return "range"; }

    virtual ostream& generate_typedef(ostream& os) const override {
        os << "typedef struct { ";
        CB_f64::type.generate_type(os);
        os << " r_start; "
        CB_f64::type.generate_type(os);
        " r_end; } ";
        generate_type(os);
        return os << ";";
    }
    virtual ostream& generate_literal(ostream& os, void const* raw_data) const override {
        ASSERT(raw_data);
        os << "(";
        generate_type(os);
        os << "){";
        double* raw_it = raw_data;
        CB_f64::type.generate_literal(os, raw_it);
        os << ", ";
        CB_f64::type.generate_literal(os, raw_it+1);
        os << "}";
    }
}


// Below: utilities version of any that can be used in c++

struct range {
    double r_start;
    double r_end; // has to have r_ prefix to not collide with end() for range-based cpp loops. In CB, call this just "end"

    std::string toS() const override {
        return r_start.toS() + ".." + r_end.toS();
    }

    struct iterator {
        double current;
        double step=1;
        const range& r;
        iterator(const range& r, double current, double step=1) : r{r}, current{current}, step{step} {}
        double operator*() const { ASSERT(current <= r.r_end); return current; }
        iterator& operator++() { current = current + step; return *this; }
        bool operator==(const iterator& o) const { return current == o.current || current > r.r_end && o.current > r.r_end; }
        bool operator!=(const iterator& o) const { return !(*this == o); }
        bool operator<(const iterator& o) const { return current < o.current; }
    };

};

static range::iterator begin(const range& r) { return range::iterator(r, r.r_start); }
static range::iterator end(const range& r) { return range::iterator(r, r.r_end + 1); }
