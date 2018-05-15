#pragma once

#include "value_expression.h"
#include "../../types/cb_struct.h"

#include <sstream>

struct Abstx_struct_literal : Value_expression {
    Owned<Abstx_scope> struct_scope;
    Shared<const CB_Type> struct_type; // has to have a value; owned by built_in_types list
    Any const_value;

    std::string toS() const override {
        ASSERT(struct_type);
        return struct_type->toS();
    }

    Shared<const CB_Type> get_type() override {
        return CB_Type::type;
    }

    bool has_constant_value() const {
        return true;
    }

    const Any& get_constant_value() override {
        if (const_value.v_ptr != nullptr) return const_value;
        const_value.v_type = CB_Type::type;
        const_value.v_ptr = &struct_type->uid;
        return const_value;
    }

    void generate_code(std::ostream& target) const override
    {
        ASSERT(is_codegen_ready(status));
        struct_type->generate_type(target);
    }

    void finalize() override {
        if (is_error(status) || is_codegen_ready(status)) return;
        ASSERT(struct_type);
        status = Parsing_status::FULLY_RESOLVED;
    }
};

