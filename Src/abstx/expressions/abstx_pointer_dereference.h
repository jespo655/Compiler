#pragma once

#include "variable_expression.h"

struct Abstx_identifier;

struct Abstx_pointer_dereference : Variable_expression {

    Shared<Abstx_identifier> pointer_id; // Owned by parent scope

    std::string toS() const override;

    virtual Shared<const CB_Type> get_type() override;

    void generate_code(std::ostream& target, const Token_context& context) const override;

};


struct Abstx_address_of : Value_expression {

    Shared<Abstx_identifier> pointer_id; // Owned by parent scope
    Shared<const CB_Type> pointer_type = nullptr;

    std::string toS() const override;

    virtual Shared<const CB_Type> get_type() override;

    void generate_code(std::ostream& target, const Token_context& context) const override;

};
