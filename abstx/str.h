#pragma once

#include "type.h"

struct Type_str : Type
{
    std::string toS() const override { return "str"; }
    std::shared_ptr<const Literal> get_default_value() const override;
};


#include "literal.h"

struct Literal_str : Literal
{
    std::string value = "";
    std::string toS() const override { return '\"' + value + '\"'; }
    std::shared_ptr<const Type> get_type() const override;
};
