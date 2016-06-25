#pragma once

#include "evaluated_value.h"
#include <string>



// top level type class
// abstract, only used as a node in the abstx tree
struct Type : Evaluated_value
{
    std::shared_ptr<Type> get_type() override; // should return Type_type for all types
    virtual std::string toS(void const * value_ptr, int size=0) const = 0; // treat the contents of the pointer as a member of this type. If size=0, ignore it.
    std::string toS() const override = 0;


    virtual int byte_size() const = 0; // should return the size of the type in bytes.

    virtual bool operator==(const Type& o) const {
        return toS() == o.toS() && *context == *o.context;
    };
    virtual bool operator!=(const Type& o) const { return !(*this==o); }
};


// The type of all types
struct Type_type : Type
{
    std::string toS(void const * value_ptr, int size=0) const override { ASSERT(false); } // this should never be called

    std::string toS() const override { return "type"; }
    int byte_size() const override { return 0; } // all info about types are handled at compile time, so no alloc needed at runtime
};

