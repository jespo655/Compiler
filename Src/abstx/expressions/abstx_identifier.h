#pragma once

#include "variable_expression.h"
#include "../../types/cb_any.h"

#include <sstream>

struct Abstx_identifier : Variable_expression {
    std::string name = "";
    Any value; // might not have an actual value, but must have a type. Constant declared identifiers must have a value
    uint64_t uid = 0; // = get_unique_id(); // set to 0 to avoid using suffixes

    // pointer to the value expression that defined this identifier, or nullptr if not applicable
    // This can be non-standard CB values, for example be a function or scope expression
    Shared<Value_expression> value_expression = nullptr;

    Abstx_identifier(const std::string& name="", Shared<const CB_Type> type=nullptr, void const* value=nullptr) : name{name}, value{type, value} {}

    std::string toS() const override;

    // get the type of this identifier
    Shared<const CB_Type> get_type() override;

    // get the type of this identifier when it has a constant value
    Shared<const CB_Type> get_constant_type() const;

    // check if this identifier has a constant value
    bool has_constant_value() const override;

    // get the constant value of this identifier if it has one
    const Any& get_constant_value() override;

    // generate c code
    void generate_code(std::ostream& target, const Token_context& context) const override;

    // finalize this expression, trying to set type and value of this identifier
    void finalize() override;

};
