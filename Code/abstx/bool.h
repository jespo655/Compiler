#pragma once

#include "type.h"

struct Type_bool : Type
{
    std::string toS() const override { return "bool"; }
    int byte_size() const override { return sizeof(bool); }

    std::string toS(void const * value_ptr, int size=0) const override {
        ASSERT(size == 0 || size == byte_size())
        bool b = *(bool*)value_ptr;
        return b ? "true" : "false";
    }

    static bool cpp_value(void const* value_ptr, int size=0)
    {
        ASSERT(size == 0 || size == sizeof(bool));
        ASSERT(value_ptr != nullptr);
        return *(bool*)value_ptr;
    }

};
