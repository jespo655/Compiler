#pragma once

#include "value_expression.h"
#include "../abstx_scope.h"
#include "../../types/cb_any.h"

#include <sstream>

struct Abstx_struct_literal : Value_expression {
    Owned<Abstx_scope> struct_scope;
    Shared<const CB_Type> struct_type; // has to have a value; owned by built_in_types list
    Any const_value;

    std::string toS() const override;

    Shared<const CB_Type> get_type() override;
    bool has_constant_value() const;
    const Any& get_constant_value() override;

    void generate_code(std::ostream& target) const override;
    void finalize() override;
};
