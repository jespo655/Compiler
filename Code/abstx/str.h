#pragma once

#include "type.h"

struct Type_str : Type
{
    std::string toS() const override { return "str"; }
    std::shared_ptr<Literal> get_default_value() const override;

    int byte_size() override { return sizeof(uint_least64_t) + sizeof(void*); } // fat pointer - also stores its own size
};


#include "literal.h"

struct Literal_str : Literal
{
    std::string value = "";
    std::string toS() const override { return '\"' + value + '\"'; }
    std::shared_ptr<Type> get_type() override;
};
