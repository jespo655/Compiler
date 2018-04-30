#pragma once

#include "type.h"
#include <cstring> // memcpy

/*
The type Any can be assigned a value of any type. The type information is then stored alongside the object.
The value is expected to come from and be handled by a CB context, so no destructor is called when the CB_Any is destroyed.

Usages:
    default values for types
    container for inputdata from cb functions
*/

struct CB_Any : CB_Type {
    static CB_Type type; // type any
    static const bool primitive = false;

    CB_Any() { uid = type.uid; }
    std::string toS() const override { return "any"; }

    // code generation functions
    virtual ostream& generate_typedef(ostream& os) const override {
        os << "typedef struct { ";
        CB_Type::type.generate_type(os);
        os << "type; void* v_ptr; } ";
        generate_type(os);
        return os << ";";
    }
    virtual ostream& generate_literal(ostream& os, void const* raw_data) const override {
        ASSERT(raw_data);
        os << "(";
        generate_type(os);
        os << "){";
        CB_Type::type.generate_literal(os, raw_data);
        uint8_t* raw_it = raw_data;
        raw_it += CB_Type::type.cb_sizeof();
        os << ", " << hex << (void**)raw_it << "}";
    }

}


// Below: utilities version of any that can be used in c++

struct any {
    CB_Type v_type; // the type of v
    void* v_ptr = nullptr;

    any() {} // default value
    any(const CB_Type& type, void* ptr) : v_type{type}, v_ptr{ptr} {} // default value

    std::string toS() const {
        if (v_ptr) {
            ostringstream oss;
            oss << "any(";
            v_type.generate_literal(v_ptr);
            oss << ")";
            return oss.str();
        }
        else return "any(void)";
    }

    any& operator=(const any& any) {
        v_ptr = any.v_ptr;
        v_type = any.v_type;
        memcpy(v_ptr, any.v_ptr, v_type.byte_size()); // @warning this might not be safe
    }
    any(const any& any) { *this = any; }

    any& operator=(any&& any) {
        v_type = any.v_type;
        v_ptr = any.v_ptr;
        any.v_ptr = nullptr;
    }
    any(any&& any) { *this = std::move(any); }

    bool has_value(const CB_Type& t) const {
        return t == v_type && v_ptr != nullptr;
    }
};




