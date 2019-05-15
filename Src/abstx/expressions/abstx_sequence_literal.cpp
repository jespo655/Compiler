#include "abstx_sequence_literal.h"
#include "../../types/cb_seq.h"

std::string Abstx_sequence_literal::toS() const {
    return value.toS();
}

Shared<const CB_Type> Abstx_sequence_literal::get_type() {
    if (constant_value.v_type != nullptr) return constant_value.v_type;
    if (member_type == nullptr) {
        if (value.size == 0) return nullptr; // empty sequence literal is not allowed and should be checked for earlier
        member_type = value[0]->get_type();
    }
    if (member_type == nullptr) return nullptr; // unable to get type from first value
    constant_value.v_type = CB_Seq::get_seq_type(member_type);
    return constant_value.v_type;
}

bool Abstx_sequence_literal::has_constant_value() const {
    if (constant_value.v_ptr != nullptr) return true;
    if (is_error(status)) return false;
    ASSERT(member_type != nullptr);
    for (const auto& v : value) {
        if (!v->has_constant_value()) return false;
        if (*v->get_type() != *member_type) return false;
    }
    return true;
}

const Any& Abstx_sequence_literal::get_constant_value() {
    if (constant_value.v_ptr != nullptr || !has_constant_value()) return constant_value;
    constant_value.v_ptr = alloc_constant_data(value.size * member_type->cb_sizeof());
    uint8_t* raw_it = (uint8_t*)constant_value.v_ptr;
    size_t mem_size = member_type->cb_sizeof();
    for (const auto& v : value) {
        memcpy(raw_it, v->get_constant_value().v_ptr, mem_size);
        raw_it += mem_size;
    }
    return constant_value;
}

void Abstx_sequence_literal::generate_code(std::ostream& target, const Token_context& context) const
{
    ASSERT(is_codegen_ready(status));
    if (has_constant_value()) {
        constant_value.generate_literal(target, context);
    } else {

    }
    // value.generate_literal(target);
    // @TODO: check
}
