#pragma once

#include "cb_type.h"
#include "../utilities/pointers.h"

#include <cstring> // memcpy
#include <iomanip> // hex
#include <sstream> // ostringstream

/*
The type Any can be assigned a value of any type. The type information is then stored alongside the object.
The value is expected to come from and be handled by a CB context, so no destructor is called when the CB_Any is destroyed.

Usages:
    default values for types
    container for inputdata from cb functions
*/

struct CB_Any : CB_Type {
    static const Shared<const CB_Type> type; // type any
    static constexpr void* _default_value = nullptr;

    CB_Any() { uid = type->uid; }
    CB_Any(const std::string& name, size_t size, void const* default_value) : CB_Type(name, size, default_value) {}
    std::string toS() const override { return "any"; }

    bool is_primitive() const override { return false; }

    void generate_type(ostream& os) const override { os << "_cb_any"; }

    // code generation functions
    void generate_typedef(ostream& os) const override {
        os << "typedef struct { ";
        CB_Type::type->generate_type(os);
        os << " type; void* v_ptr; } ";
        generate_type(os);
        os << ";" << std::endl;
    }
    void generate_literal(ostream& os, void const* raw_data, uint32_t depth = 0) const override {
        if (depth > MAX_ALLOWED_DEPTH) { post_circular_reference_error(); os << "void"; return; }
        ASSERT(raw_data);
        os << "(";
        generate_type(os);
        os << "){";
        CB_Type::type->generate_literal(os, raw_data, depth+1);
        uint8_t const* raw_it = (uint8_t const*)raw_data;
        raw_it += CB_Type::type->cb_sizeof();
        os << ", " << std::hex << (void**)raw_it << "}";
    }
    void generate_destructor(ostream& os, const std::string& id, uint32_t depth = 0) const override {
        if (depth > MAX_ALLOWED_DEPTH) { post_circular_reference_error(); return; }
    }
};


// Below: utilities version of any that can be used in c++

struct Any {
    Shared<const CB_Type> v_type; // the type of v
    void const* v_ptr = nullptr; // the value, owned by someone else (do not delete it!)

    Any() {} // default value
    Any(Shared<const CB_Type> type, void const* ptr) : v_type{type}, v_ptr{ptr} {} // default value

    std::string toS() const {
        if (v_ptr) {
            std::ostringstream oss;
            v_type->generate_literal(oss, v_ptr);
            return oss.str();
        }
        else return "---";
    }

    Any& operator=(const Any& Any) {
        v_ptr = Any.v_ptr;
        v_type = Any.v_type;
    }
    Any(const Any& Any) { *this = Any; }

    Any& operator=(Any&& Any) {
        v_type = Any.v_type;
        v_ptr = Any.v_ptr;
        Any.v_ptr = nullptr;
    }
    Any(Any&& Any) { *this = std::move(Any); }

    bool has_value(const CB_Type& t) const {
        return t == *v_type && v_ptr != nullptr;
    }

    void generate_literal(ostream& os) const {
        v_type->generate_literal(os, v_ptr);
    }
};


// functions to parse Any data to c++ native types in a typesafe way
uint32_t parse_type_id(const Any& any);
Shared<const CB_Type> parse_type(const Any& any);
std::string parse_string(const Any& any);
int64_t parse_int(const Any& any);
uint64_t parse_uint(const Any& any);
uint64_t parse_float(const Any& any);


