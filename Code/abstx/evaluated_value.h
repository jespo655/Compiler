#pragma once

#include "abstx.h"

struct Type;

#include <string>

// An evaluated value is anything that evaluates to a value that can be read.
struct Evaluated_value : Abstx_node
{
    virtual std::shared_ptr<const Type> get_type() const = 0;
};


#include "type.h"
