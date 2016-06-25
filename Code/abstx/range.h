#pragma once

#include "type.h"

struct Type_range : Type
{
    std::string toS() const override { return "range"; }
    std::shared_ptr<Literal> get_default_value() const override;

    int byte_size() override { return sizeof(uint_least8_t); }
};


#include "literal.h"
#include "float.h"

struct Literal_range : Literal
{
    std::shared_ptr<Literal_float> start{new Literal_float()};
    std::shared_ptr<Literal_float> end{new Literal_float()};

    std::string toS() const override
    {
        ASSERT(start != nullptr);
        ASSERT(end != nullptr);
        return start->toS() + ".." + end->toS();
    }

    std::shared_ptr<Type> get_type() override;

};
