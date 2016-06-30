#pragma once

#include "value_expression.h"
#include "struct.h"
#include "scope.h"

/*
A cast changes the type of a value to another type.
It will return exactly one value.

It is equivalent to a call to the cast function
TODO: find exact syntax for how to user define casts
void _cast_int_float(int v, *float r) { *r = v; }
*/
struct Cast : Value_expression {

    std::shared_ptr<Value_expression> value_identifier;
    std::shared_ptr<Identifier> type_identifier;

    std::shared_ptr<Type> get_type() override
    {
        ASSERT(type_identifier != nullptr);
        if (type_identifier->type != nullptr) return type_identifier->type;
        auto scope = parent_scope();
        ASSERT(scope != nullptr);
        auto type = scope->get_type(type_identifier->name);

        // FIXME: ensure that there is a valid conversion from value_identifier->get_type() to type
        if (type != nullptr) fully_resolved = true;
        type_identifier->type = type;
        return type;
    }

    std::string toS() const override { return "cast"; } // FIXME: better Cast::toS()

};
