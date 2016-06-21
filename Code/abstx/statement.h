#pragma once

#include "abstx.h"


// A statement is a line of code.
// This is also the smallest unit of compilation.
struct Statement : Abstx_node
{
    virtual bool allow_in_static_scope() const = 0;
    virtual bool allow_in_dynamic_scope() const = 0;
};