#pragma once

#include "value_expression.h"
#include <string>













// top level type class
// abstract, only used as a node in the abstx tree
struct Type : Value_expression
{
    std::shared_ptr<Type> get_type() override; // should return Type_type for all types. Implemented in implementations.cpp. FIXME: move to a better place
    virtual std::string toS(void const * value_ptr, int size=0) const = 0; // treat the contents of the pointer as a member of this type. If size=0, ignore it.
    std::string toS() const override = 0;


    virtual int byte_size() const = 0; // should return the size of the type in bytes.

    virtual bool operator==(const Type& o) const {
        return toS() == o.toS() && context == o.context;
    };
    virtual bool operator!=(const Type& o) const { return !(*this==o); }

    virtual bool is_number_type() const { return false; }
    virtual bool is_integer_type() const { return false; }
    virtual bool is_float_type() const { return false; }
    virtual bool is_string_type() const { return false; }
    virtual bool is_type_type() const { return false; }
    virtual bool is_bool_type() const { return false; }
    virtual bool is_seq_type() const { return false; }
    virtual bool is_function_type() const { return false; }
    virtual bool is_operator_type() const { return false; }

};


// The type of all types
struct Type_type : Type
{
    std::string toS(void const * value_ptr, int size=0) const override { ASSERT(false); } // this should never be called

    std::string toS() const override { return "type"; }
    int byte_size() const override { return 0; } // all info about types are handled at compile time, so no alloc needed at runtime

    bool is_type_type() const override { return true; }
};

/*
Maybe:
A type has a value with an uid
That value is use for every conversion
Built in types are pre-defined with set values


static auto tt = std::shared_ptr<Type_type>(new Type_type());
tt->value.assign(tt, get_uid());

Type_type.value would then have a pointer

*/