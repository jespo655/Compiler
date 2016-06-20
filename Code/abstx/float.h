#pragma once

#include "type.h"


struct Type_f32 : Type
{
    std::string toS() const override { return "f32"; }
    std::shared_ptr<const Literal> get_default_value() const override;
};

struct Type_f64 : Type
{
    std::string toS() const override { return "f64"; }
    std::shared_ptr<const Literal> get_default_value() const override;
};

// struct Type_float : Type
// {
//     std::string toS() const { return "float"; }
// };

#define Type_float Type_f64



#include "literal.h"

struct Literal_float : Literal
{
    double value = 0.0;
    std::string toS() const override { return std::to_string(value); }
    std::shared_ptr<const Type> get_type() const override;
};




