#pragma once

#include "variable_expression.h"
#include "struct.h"
#include "scope.h"

/*
A getter grabs a data member from a scope or struct. It will return exactly one value (the value of that data member).

Some examples:
x.a; // named getter, getter_id is an identifier
{ a := 2; }.a; // direct access from a scope literal
xs[2].a; // anonymous call from an lookup from an array of functions
*/
struct Getter : Variable_expression {

    std::shared_ptr<Value_expression> getter_identifier;
    std::shared_ptr<Identifier> data_identifier;

    std::shared_ptr<Type> get_type() override
    {
        ASSERT(data_identifier != nullptr);
        auto type = data_identifier->get_type();
        if (type != nullptr) return type;

        ASSERT(getter_identifier != nullptr);
        auto getter_type = getter_identifier->get_type();
        if (auto struct_t = std::dynamic_pointer_cast<const Type_struct>(getter_type)) {
            // check for members
            auto id = struct_t->get_member(data_identifier->name);
            if (id != nullptr) type = id->type;
        }
        if (auto scope = std::dynamic_pointer_cast<Literal_scope>(getter_identifier)) {
            // check for identifiers
            auto id = scope->get_identifier(data_identifier->name);
            if (id != nullptr) type = id->type;
        }
        if (type != nullptr) {
            data_identifier->type = type;
            status = Parsing_status::FULLY_RESOLVED;
            return type;
        }
        return nullptr;
    }

    std::string toS() const override { return "getter"; } // FIXME: better Getter::toS()

};
