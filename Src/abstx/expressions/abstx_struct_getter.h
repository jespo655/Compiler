#pragma once

#include "variable_expression.h"
#include "../../types/cb_struct.h"

namespace Cube {

struct Abstx_struct_getter : Variable_expression {

    Owned<Variable_expression> struct_expr;
    std::string member_id;
    Shared<const CB_Struct::Struct_member> member; // set by get_member
    Any value; // type set by get_type; value set by constant_value

    std::string toS() const override;

    Shared<const CB_Type> get_type() override;
    bool has_constant_value() const override;
    const Any& get_constant_value() override;

    void generate_code(std::ostream& target) const override;
    void finalize() override;

private:
    bool get_member();
};

}
