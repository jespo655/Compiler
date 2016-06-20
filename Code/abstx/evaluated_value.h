#pragma once

#include "abstx.h"

struct Type;

#include <string>

// An evaluated value is anything that evaluates to a value that can be read.
struct Evaluated_value : Abstx_node
{
    // get_type() should return nullptr if unable to infer the type (yet).
    virtual std::shared_ptr<const Type> get_type() = 0;
};


#include "type.h"
