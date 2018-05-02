#pragma once

#include "../abstx.h"
#include "../../utilities/sequence.h"
#include "../../types/cb_type.h"

// An evaluated value is anything that evaluates to a value that can be read.
struct Value_expression : Abstx_node
{
    /*
        get_type() should return nullptr if unable to infer the type (yet).
    */
    virtual Shared<const CB_Type> get_type() = 0;
};


