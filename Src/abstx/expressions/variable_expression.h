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
        finalize();
        ASSERT(expr);
        return expr->get_type();
    }

    bool has_constant_value() const override {
        ASSERT(expr);
        return expr->has_constant_value();
    }

    void const* get_constant_value() override {
        ASSERT(expr);
        return expr->get_constant_value();
    }

    Parsing_status fully_parse() override {
        if (status != Parsing_status::PARTIALLY_PARSED) return status;
        ASSERT(expr);
        status = Parsing_status::FULLY_PARSED;
        return status;
    }

    Parsing_status finalize() override {
        ASSERT(expr);
        if (is_error(expr->status)) status = expr->status;
        else status = Parsing_status::FULLY_RESOLVED;
        return status;
    }

    void generate_code(std::ostream& target) override {
        finalize();
        return expr->generate_code(target);
    }
};


