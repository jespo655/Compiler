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

struct CB_Iterable {
    // generate_for(): called to output c-style for loop (including "for" and parens)
    // after this, a scope (including braces) will be printed
    // for now, only accept positive integer steps
    virtual void generate_for(ostream& os, const std::string& id, const std::string& it_name = "it", uint64_t step = 1, bool reverse = false, bool protected_scope = true) const = 0;

    // generate_for_after_scope(): called after the for scope is printed (directly after the closing })
    // this function is optional to implement
    virtual void generate_for_after_scope(ostream& os, bool protected_scope = true) const {}
};

struct CB_Range : CB_Type, CB_Iterable {
    static const Shared<const CB_Type> type;
    static constexpr int64_t _default_value[2] = {0, 0};

    CB_Range() { uid = type->uid; }
    CB_Range(const std::string& name, size_t size, void const* default_value) : CB_Type(name, size, default_value) {}
    std::string toS() const override { return "range"; }

    bool is_primitive() const override { return false; }

    virtual size_t alignment() const override { return CB_i64::type->alignment(); }

    void generate_type(ostream& os) const override { os << "_cb_i_range"; }

    void generate_typedef(ostream& os) const override {
        os << "typedef struct { ";
        CB_i64::type->generate_type(os);
        os << " r_start; ";
        CB_i64::type->generate_type(os);
        os << " r_end; } ";
        generate_type(os);
        os << ";" << std::endl;
    }
    void generate_literal(ostream& os, void const* raw_data, uint32_t depth = 0) const override {
        ASSERT(raw_data);
        os << "(";
        generate_type(os);
        os << "){";
        int64_t const* raw_it = (int64_t const*)raw_data;
        CB_i64::type->generate_literal(os, raw_it, depth+1);
        os << ", ";
        CB_i64::type->generate_literal(os, raw_it+1, depth+1);
        os << "}";
    }
    void generate_for(ostream& os, const std::string& id, const std::string& it_name = "it", uint64_t step = 1, bool reverse = false, bool protected_scope = true) const override {
        os << "for (";
        CB_i64::type->generate_type(os);
        os << " " << it_name << " = " << id << (reverse?".r_end":".r_start") << "; ";
        os << it_name << (reverse?" >= ":" <= ") << id << (reverse?".r_start":".r_end") << ";";
        os << it_name << (reverse?" -= ":" += ") << step << ")";
    }
};


struct CB_Float_range : CB_Type, CB_Iterable {
    static const Shared<const CB_Type> type;
    static constexpr double _default_value[2] = {0, 0};

    CB_Float_range() { uid = type->uid; }
    CB_Float_range(const std::string& name, size_t size, void const* default_value) : CB_Type(name, size, default_value) {}
    std::string toS() const override { return "float_range"; }

    bool is_primitive() const override { return false; }

    virtual size_t alignment() const override { return CB_f64::type->alignment(); }

    void generate_type(ostream& os) const override { os << "_cb_f_range"; }

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
    void generate_for(ostream& os, const std::string& id, const std::string& it_name = "it", uint64_t step = 1, bool reverse = false, bool protected_scope = true) const override {
        os << "for (";
        CB_f64::type->generate_type(os);
        os << " " << it_name << " = " << id << (reverse?".r_end":".r_start") << "; ";
        os << it_name << (reverse?" >= ":" <= ") << id << (reverse?".r_start":".r_end") << ";";
        os << it_name << (reverse?" -= ":" += ") << step << ")";
    }
};


// Below: utilities version of any that can be used in c++

struct range {
    int64_t r_start;
    int64_t r_end; // has to have r_ prefix to not collide with end() for range-based cpp loops. In CB, call this just "end"

    std::string toS() const {
        std::ostringstream oss;
        oss << r_start << ".." << r_end;
        return oss.str();
    }

    struct iterator {
        int64_t current;
        int64_t step=1;
        const range& r;
        iterator(const range& r, int64_t current, int64_t step=1) : r{r}, current{current}, step{step} {}
        int64_t operator*() const { ASSERT(current <= r.r_end); return current; }
        iterator& operator++() { current = current + step; return *this; }
        bool operator==(const iterator& o) const { return current == o.current || current > r.r_end && o.current > r.r_end; }
        bool operator!=(const iterator& o) const { return !(*this == o); }
        bool operator<(const iterator& o) const { return current < o.current; }
    };

};

static range::iterator begin(const range& r) { return range::iterator(r, r.r_start); }
static range::iterator end(const range& r) { return range::iterator(r, r.r_end + 1); }



struct float_range {
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
        const float_range& r;
        iterator(const float_range& r, double current, double step=1) : r{r}, current{current}, step{step} {}
        double operator*() const { ASSERT(current <= r.r_end); return current; }
        iterator& operator++() { current = current + step; return *this; }
        bool operator==(const iterator& o) const { return current == o.current || current > r.r_end && o.current > r.r_end; }
        bool operator!=(const iterator& o) const { return !(*this == o); }
        bool operator<(const iterator& o) const { return current < o.current; }
    };

};

static float_range::iterator begin(const float_range& r) { return float_range::iterator(r, r.r_start); }
static float_range::iterator end(const float_range& r) { return float_range::iterator(r, r.r_end + 1); }
