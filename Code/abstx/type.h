#pragma once

#include "evaluated_value.h"

struct Literal;

// top level type class
// abstract, only used as a node in the abstx tree
struct Type : Evaluated_value
{
    virtual std::shared_ptr<const Literal> get_default_value() const { return nullptr; } // return nullptr if the type has no default value
    std::shared_ptr<const Type> get_type() const override; // should return Type_type for all types

    virtual bool operator==(const Type& o) const {
        return toS() == o.toS() && *context == *o.context;
    };
    virtual bool operator!=(const Type& o) const { return !(*this==o); }
};


// The type of all types
struct Type_type : Type
{
    std::string toS() const override { return "type"; }
};



