#include "abstx_struct_literal.h"
// #include "../../types/cb_struct.h" // @todo check if this is necessary

std::string Abstx_struct_literal::toS() const {
    ASSERT(struct_type);
    return struct_type->toS();
}

Shared<const CB_Type> Abstx_struct_literal::get_type() {
    return CB_Type::type;
}

bool Abstx_struct_literal::has_constant_value() const {
    return true;
}

const Any& Abstx_struct_literal::get_constant_value() {
    if (const_value.v_ptr != nullptr) return const_value;
    const_value.v_type = CB_Type::type;
    const_value.v_ptr = &struct_type->uid;
    return const_value;
}

void Abstx_struct_literal::generate_code(std::ostream& target, const Token_context& context) const
{
    ASSERT(is_codegen_ready(status));
    struct_type->generate_type(target);
}

void Abstx_struct_literal::finalize() {
    if (is_error(status) || is_codegen_ready(status)) return;
    ASSERT(struct_type);
    status = Parsing_status::FULLY_RESOLVED;
}
