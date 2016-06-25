#pragma once

#include "type.h"

struct Type_bool : Type
{
    std::string toS() const override { return "bool"; }
    int byte_size() const override { return sizeof(uint_least8_t); }
};
