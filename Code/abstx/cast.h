#pragma once

#include "evaluated_value.h"
#include "struct.h"
#include "scope.h"

/*
A cast changes the type of a value to another type.
It will return exactly one value.
*/
struct Cast : Evaluated_value {

    std::shared_ptr<Evaluated_value> value_identifier;
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
