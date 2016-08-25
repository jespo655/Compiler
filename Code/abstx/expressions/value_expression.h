#pragma once

#include "../abstx.h"

// An evaluated value is anything that evaluates to a value that can be read.
struct Value_expression : Abstx_node
{
    // get_type() should return nullptr if unable to infer the type (yet).
    virtual shared<CB_Type> get_type() = 0;
};


