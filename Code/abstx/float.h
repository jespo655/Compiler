#pragma once

#include "type.h"


struct Type_f32 : Type
{
    std::string toS() const override { return "f32"; }
    int byte_size() const override { return sizeof(float); }
};

struct Type_f64 : Type
{
    std::string toS() const override { return "f64"; }
    int byte_size() const override { return sizeof(double); }
};

// struct Type_float : Type
// {
//     std::string toS() const { return "float"; }
// };

#define Type_float Type_f64



