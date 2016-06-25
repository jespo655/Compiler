#pragma once

#include "evaluated_value.h"

struct Literal;

// top level type class
// abstract, only used as a node in the abstx tree
struct Type : Evaluated_value
{
    virtual std::shared_ptr<Literal> get_default_value() const { return nullptr; } // return nullptr if the type has no default value
    std::shared_ptr<Type> get_type() override; // should return Type_type for all types

    virtual int byte_size() = 0; // should return the size of the type in bytes.

    virtual bool operator==(const Type& o) const {
        return toS() == o.toS() && *context == *o.context;
    };
    virtual bool operator!=(const Type& o) const { return !(*this==o); }
};


// The type of all types
struct Type_type : Type
{
    std::string toS() const override { return "type"; }
    int byte_size() override { return 0; } // all info about types are handled at compile time, so no alloc needed at runtime
};



