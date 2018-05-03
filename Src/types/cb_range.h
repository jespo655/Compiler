#pragma once

#include "cb_type.h"
#include "cb_primitives.h"

#include <string>
#include <sstream>

/*
Range - a compact data structure with a start and an end.
Can be iterated over

Syntax:
a : Range = 0..2;
*/

struct CB_Range : CB_Type {
    static const Shared<const CB_Type> type;
    static constexpr double _default_value[2] = {0, 0};

    CB_Range() { uid = type->uid; }
    CB_Range(const std::string& name, size_t size, void const* default_value) : CB_Type(name, size, default_value) {}
    std::string toS() const override { return "range"; }

    bool is_primitive() const override { return false; }

    virtual size_t alignment() const override { return CB_f64::type->alignment(); }

    void generate_type(ostream& os) const override { os << "_cb_range"; }

    void generate_typedef(ostream& os) const override {
        os << "typedef struct { ";
        CB_f64::type->generate_type(os);
        os << " r_start; ";
        CB_f64::type->generate_type(os);
        os << " r_end; } ";
        generate_type(os);
        os << ";" << std::endl;
    }
    void generate_literal(ostream& os, void const* raw_data, uint32_t depth = 0) const override {
        ASSERT(raw_data);
        os << "(";
        generate_type(os);
        os << "){";
        double const* raw_it = (double const*)raw_data;
        CB_f64::type->generate_literal(os, raw_it, depth+1);
        os << ", ";
        CB_f64::type->generate_literal(os, raw_it+1, depth+1);
        os << "}";
    }
};


// Below: utilities version of any that can be used in c++

struct range {
    double r_start;
    double r_end; // has to have r_ prefix to not collide with end() for range-based cpp loops. In CB, call this just "end"

    std::string toS() const {
        std::ostringstream oss;
        oss << r_start << ".." << r_end;
        return oss.str();
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
