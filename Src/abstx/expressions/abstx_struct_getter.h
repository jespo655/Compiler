#pragma once

#include "variable_expression.h"
#include "abstx_identifier.h"
#include "../../types/cb_struct.h"

struct Abstx_struct_getter : Variable_expression {

    Owned<Variable_expression> struct_expr;
    std::string member_id;
    Shared<const CB_Struct::Struct_member> member; // set by get_member
    Any value; // type set by get_type; value set by constant_value

    std::string toS() const override {
        ASSERT(member_id.length() > 0);
        std::ostringstream oss;
        oss << struct_expr.toS() << "." << member_id;
        return oss.str();
    }

    Shared<const CB_Type> get_type() override {
        if (value.v_type != nullptr) return value.v_type;
        if (!get_member()) return nullptr;
        value.v_type = member->id.get_constant_type();
        return value.v_type;
    }

    bool has_constant_value() const override {
        if (value.v_ptr != nullptr) return true;
        if (member == nullptr) return false; // should be set during finalize
        return struct_expr != nullptr && struct_expr->has_constant_value();
    }

    const Any& get_constant_value() override {
        if (value.v_type == nullptr) get_type();
        if (value.v_ptr != nullptr) return value;
        ASSERT(member != nullptr); // should be set by get_type() above
        const Any& struct_defval = struct_expr->get_constant_value();
        ASSERT(struct_defval.v_ptr != nullptr); // if this is the case, has_constant_value() should return false and this function should never be run
        value.v_ptr = &((uint8_t*)struct_defval.v_ptr)[member->byte_position];
        return value;
    }

    void generate_code(std::ostream& target) const override
    {
        ASSERT(is_codegen_ready(status));
        struct_expr->generate_code(target);
        target << ".";
        member->id.generate_code(target);
    }

    void finalize() override {
        if (is_error(status) || is_codegen_ready(status)) return;
        if (get_type()) {
            status = Parsing_status::FULLY_RESOLVED;
        } else {
            status = Parsing_status::DEPENDENCIES_NEEDED;
        }
    }

private:

    bool get_member() {
        if (member == nullptr) {
            ASSERT(struct_expr);
            Shared<const CB_Type> type = struct_expr->get_type();
            Shared<const CB_Struct> struct_type = dynamic_pointer_cast<const CB_Struct>(type);
            if (struct_type == nullptr) return false;
            member = struct_type->get_member(member_id);
        }
        return member != nullptr;
    }
};
