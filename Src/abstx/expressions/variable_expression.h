#pragma once

#include "value_expression.h"

// An evaluated variable is anything that can be assigned a value.
struct Variable_expression : Value_expression
{

};



struct Variable_expression_reference : Variable_expression {
    Shared<Variable_expression> expr = nullptr;

    std::string toS() const override {
        ASSERT(expr);
        return expr->toS();
    }

    virtual Shared<const CB_Type> get_type() override {
        ASSERT(expr);
        return expr->get_type();
    }

    bool has_constant_value() const override {
        ASSERT(expr);
        return expr->has_constant_value();
    }

    const Any& get_constant_value() override {
        ASSERT(expr);
        return expr->get_constant_value();
    }

    void generate_code(std::ostream& target) override {
        ASSERT(expr);
        return expr->generate_code(target);
    }
};


