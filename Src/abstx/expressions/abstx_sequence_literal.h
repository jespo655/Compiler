#pragma once

#include "value_expression.h"
#include "../../types/cb_type.h"

struct Abstx_sequence_literal : Value_expression {

    Seq<Owned<Value_expression>> value;
    Shared<const CB_Type> member_type; // if null, the value is given by the first
    Any constant_value; // type set automatically when get_type() is called (stored so we don't have to redo the work)

    std::string toS() const override;

    virtual Shared<const CB_Type> get_type() override;
    bool has_constant_value() const override;
    const Any& get_constant_value() override;

    void generate_code(std::ostream& target) const override;
};
