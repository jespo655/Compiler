#pragma once

#include "../abstx_node.h"
#include "../../utilities/sequence.h"
#include "../../types/cb_type.h"
#include "../../types/cb_any.h"

namespace Cube {

// An evaluated value is anything that evaluates to a value that can be read.
struct Value_expression : Abstx_node
{
    // get_type(): should return nullptr if unable to infer the type (yet).
    virtual Shared<const CB_Type> get_type() = 0;

    // has_constant_value(): return true if there is a constant value
    virtual bool has_constant_value() const = 0;

    // get_constant_value(): return nullpointer if there is no constant value
    // this value can then be parsed by the type (given by get_type())
    virtual const Any& get_constant_value() = 0;

    // finalize(): try to go from non-error parsing status to FULLY_RESOLVED
    // this might require to finalized owned members or even shared dependencies
    virtual void finalize() = 0;
};

}