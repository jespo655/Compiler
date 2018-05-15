#pragma once

#include "value_expression.h"
#include "../../types/cb_type.h"
#include "../../types/cb_seq.h"

struct Abstx_sequence_literal : Value_expression {

    Seq<Owned<Value_expression>> value;
    Shared<const CB_Type> member_type; // if null, the value is given by the first
    Any constant_value; // type set automatically when get_type() is called (stored so we don't have to redo the work)

    std::string toS() const override {
        return value.toS();
    }

    virtual Shared<const CB_Type> get_type() override {
        if (constant_value.v_type != nullptr) return constant_value.v_type;
        if (member_type == nullptr) {
            if (value.size == 0) return nullptr; // empty sequence literal is not allowed and should be checked for earlier
            member_type = value[0]->get_type();
        }
        if (member_type == nullptr) return nullptr; // unable to get type from first value
        constant_value.v_type = CB_Seq::get_seq_type(member_type);
        return constant_value.v_type;
    }

    bool has_constant_value() const override {
        if (constant_value.v_ptr != nullptr) return true;
        if (is_error(status)) return false;
        ASSERT(member_type != nullptr);
        for (const auto& v : value) {
            if (!v->has_constant_value()) return false;
            if (*v->get_type() != *member_type) return false;
        }
        return true;
    }

    const Any& get_constant_value() override {
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

    void generate_code(std::ostream& target) const override
    {
        ASSERT(is_codegen_ready(status));
        if (has_constant_value()) {
            constant_value.generate_literal(target);
        } else {

        }
        // value.generate_literal(target);
    }
};

