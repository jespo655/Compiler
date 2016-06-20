#pragma once

#include "type.h"

struct Type_bool : Type
{
    std::string toS() const override { return "bool"; }
    std::shared_ptr<const Literal> get_default_value() const override;
};


#include "literal.h"

struct Literal_bool : Literal
{
    bool value = false;
    std::string toS() const override { return value ? "true" : "false"; }
    std::shared_ptr<const Type> get_type() const override;
};
