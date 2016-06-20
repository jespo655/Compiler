#pragma once

#include "type.h"
#include "identifier.h"

#include <sstream>
#include <vector>

// Structs has no literals.
// They should instead be assigned to through constructors or getters.

struct Type_struct : Type
{
    std::vector<std::shared_ptr<Identifier>> members; // can be empty

    std::string toS() const override {
        std::ostringstream oss;
        oss << "struct{";
        bool first = true;
        for (const auto id : members) {
            ASSERT(id != nullptr);
            if (!first) oss << "; ";
            oss << id->toS();
            first = false;
        }
        oss << "}";
        return oss.str();
    }

    std::shared_ptr<const Literal> get_default_value() const override
    {
        ASSERT(false, "Type_struct::get_default_value() should never be called. Set default values recursively for each member instead.");
        return nullptr;
    }
};