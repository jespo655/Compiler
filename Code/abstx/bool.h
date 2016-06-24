#pragma once

#include "type.h"

struct Type_bool : Type
{
    std::string toS() const override { return "bool"; }
    std::shared_ptr<const Literal> get_default_value() const override;

    int byte_size() override { return sizeof(uint_least8_t); }
};


#include "literal.h"

struct Literal_bool : Literal
{
    bool value = false;
    std::string toS() const override { return value ? "true" : "false"; }
    std::shared_ptr<Type> get_type() override;
};
