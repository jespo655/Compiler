#pragma once

#include "abstx.h"

struct Type;

// An evaluated value is anything that evaluates to a value that can be read.
struct Value_expression : Abstx_node
{
    // get_type() should return nullptr if unable to infer the type (yet).
    virtual std::shared_ptr<Type> get_type() = 0;
};


