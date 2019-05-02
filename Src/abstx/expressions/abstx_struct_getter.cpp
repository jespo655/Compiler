#include "abstx_struct_getter.h"

namespace Cube {


std::string Abstx_struct_getter::toS() const override {
    ASSERT(member_id.length() > 0);
    std::ostringstream oss;
    oss << struct_expr.toS() << "." << member_id;
    return oss.str();
}

Shared<const CB_Type> Abstx_struct_getter::get_type() override {
    if (value.v_type != nullptr) return value.v_type;
    if (!get_member()) return nullptr;
    value.v_type = member->id->get_constant_type();
    return value.v_type;
}

bool Abstx_struct_getter::has_constant_value() const override {
    if (value.v_ptr != nullptr) return true;
    if (member == nullptr) return false; // should be set during finalize
    return struct_expr != nullptr && struct_expr->has_constant_value();
}

const Any& Abstx_struct_getter::get_constant_value() override {
    if (value.v_type == nullptr) get_type();
    if (value.v_ptr != nullptr) return value;
    ASSERT(member != nullptr); // should be set by get_type() above
    const Any& struct_defval = struct_expr->get_constant_value();
    ASSERT(struct_defval.v_ptr != nullptr); // if this is the case, has_constant_value() should return false and this function should never be run
    value.v_ptr = &((uint8_t*)struct_defval.v_ptr)[member->byte_position];
    return value;
}

void Abstx_struct_getter::generate_code(std::ostream& target) const override
{
    ASSERT(is_codegen_ready(status));
    struct_expr->generate_code(target);
    target << ".";
    member->id->generate_code(target);
}

void Abstx_struct_getter::finalize() override {
    if (is_error(status) || is_codegen_ready(status)) return;
    if (get_type()) {
        status = Parsing_status::FULLY_RESOLVED;
    } else {
        status = Parsing_status::DEPENDENCIES_NEEDED;
    }
}

bool Abstx_struct_getter::get_member() {
    if (member == nullptr) {
        ASSERT(struct_expr);
        Shared<const CB_Type> type = struct_expr->get_type();
        Shared<const CB_Struct> struct_type = dynamic_pointer_cast<const CB_Struct>(type);
        if (struct_type == nullptr) return false;
        member = struct_type->get_member(member_id);
    }
    return member != nullptr;
}


}
