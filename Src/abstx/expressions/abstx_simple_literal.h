#pragma once

#include "value_expression.h"
#include "../../types/cb_any.h"

#include <sstream>

struct Abstx_simple_literal : Value_expression {
    Any value; // has to have a value

    std::string toS() const override;

    Shared<const CB_Type> get_type() override;
    bool has_constant_value() const;
    const Any& get_constant_value() override;

    void generate_code(std::ostream& target, const Token_context& context) const override;
    void finalize() override;
};
