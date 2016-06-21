#pragma once

#include "type.h"
#include "identifier.h"

#include <sstream>
#include <map>

// Structs has no literals.
// They should instead be assigned to through constructors or getters.

struct Type_struct : Type
{
    std::map<std::string, std::shared_ptr<Identifier>> members; // can be empty

    std::string toS() const override
    {
        std::ostringstream oss;
        oss << "struct{";
        bool first = true;
        for (const auto id_pair : members) {
            auto id = id_pair.second;
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

    std::shared_ptr<Identifier> get_member(std::string name) const
    {
        return members.at(name);
    }
};


// TODO: struct class that holds a static scope -> that way it has access to all the special good stuff that scopes has