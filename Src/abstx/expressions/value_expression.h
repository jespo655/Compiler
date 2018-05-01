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
    virtual seq<shared<const CB_Type>> get_type() = 0;

    /*
        eval() should simplify the expression as much as possible (run time), and
        return one or more new Value_expressions containing the simplified result.
        If no simplification is possible, this method should return a deep copy of the expression.
    */
    virtual seq<owned<Value_expression>> eval() = 0;
};


