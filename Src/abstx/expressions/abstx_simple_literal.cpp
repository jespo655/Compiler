#include "abstx_simple_literal.h"

std::string Abstx_simple_literal::toS() const {
    std::ostringstream oss;
    oss << "(" << value.v_type->toS() << ")" << value.toS();
    return oss.str();
}


Shared<const CB_Type> Abstx_simple_literal::get_type() {
    return value.v_type;
}

bool Abstx_simple_literal::has_constant_value() const {
    ASSERT(value.v_ptr != nullptr); // it should be set during creation
    return true;
}

const Any& Abstx_simple_literal::get_constant_value() {
    return value;
}

void Abstx_simple_literal::generate_code(std::ostream& target) const
{
    ASSERT(is_codegen_ready(status));
    value.generate_literal(target);
}

void Abstx_simple_literal::finalize() {
    if (is_error(status) || is_codegen_ready(status)) return;
    ASSERT(value.v_type != nullptr, "type must be set during creation");
    ASSERT(value.v_ptr != nullptr, "constant value must be set during creation");
    status = Parsing_status::FULLY_RESOLVED;
}
