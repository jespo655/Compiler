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

    // code generation functions
    void generate_type(std::ostream& os) const override;
    void generate_typedef(std::ostream& os) const override;
    void generate_literal(std::ostream& os, void const* raw_data, uint32_t depth = 0) const override;
    void generate_destructor(std::ostream& os, const std::string& id, uint32_t depth = 0) const override;
};

// Below: utilities version of any that can be used in c++
struct Any : Serializable {
    Shared<const CB_Type> v_type; // the type of v
    void const* v_ptr = nullptr; // the value, owned by someone else (do not delete it!)

    Any(Shared<const CB_Type> type=nullptr, void const* ptr=nullptr) : v_type{type}, v_ptr{ptr} {} // default value

    std::string toS() const {
        if (v_ptr) {
            ASSERT(v_type);
            std::ostringstream oss;
            v_type->generate_literal(oss, v_ptr);
            return oss.str();
        }
        else return "---";
    }

    template<typename T>
    bool operator==(const T& o) const { return v_ptr && *(T*)v_ptr == o; }
    bool operator==(const Any& o) const { return v_type && *v_type == *o.v_type && memcmp(v_ptr, o.v_ptr, v_type->cb_sizeof()); }
    template<typename T>
    bool operator!=(const T& o) const { return !(*this == o); }

    Any& operator=(const Any& Any) {
        if (this != &Any) {
            v_ptr = Any.v_ptr;
            v_type = Any.v_type;
        }
        return *this;
    }
    Any(const Any& Any) { *this = Any; }

    Any& operator=(Any&& Any) {
        if (this != &Any) {
            v_type = Any.v_type;
            v_ptr = Any.v_ptr;
            Any.v_ptr = nullptr;
        }
        return *this;
    }
    Any(Any&& Any) { *this = std::move(Any); }

    bool has_value(const CB_Type& t) const {
        return t == *v_type && v_ptr != nullptr;
    }

    void generate_literal(std::ostream& os) const {
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
