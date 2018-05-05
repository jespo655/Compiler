#pragma once

#include "value_expression.h"
#include "../../types/cb_type.h"
#include "../../types/cb_seq.h"

struct Abstx_sequence_literal : Value_expression {

    Seq<Owned<Value_expression>> value;
    Shared<const CB_Type> member_type; // if null, the value is given by the first
    Shared<const CB_Type> seq_type; // set automatically when get_type() is called (stored so we don't have to redo the work)
    void* constant_value; // owned by someone else

    std::string toS() const override {
        return value.toS();
    }

    virtual Shared<const CB_Type> get_type() override {
        if (seq_type != nullptr) return seq_type;
        if (member_type == nullptr) member_type = value[0]->get_type();
        ASSERT(member_type != nullptr); // empty sequence literal is not allowed and should be checked for earlier
        seq_type = CB_Seq::get_seq_type(member_type);
        return seq_type;
    }

    bool has_constant_value() const override {
        if (constant_value != nullptr) return true;
        if (is_error(status)) return false;
        ASSERT(member_type != nullptr);
        for (const auto& v : value) {
            if (!v->has_constant_value()) return false;
            if (*v->get_type() != *member_type) return false;
        }
        return true;
    }

    void const* get_constant_value() override {
        if (constant_value != nullptr) return constant_value;
        if (!has_constant_value()) return nullptr;
        constant_value = alloc_constant_data(value.size * member_type->cb_sizeof());
        uint8_t* raw_it = (uint8_t*)constant_value;
        size_t mem_size = member_type->cb_sizeof();
        for (const auto& v : value) {
            memcpy(raw_it, v->get_constant_value(), mem_size);
            raw_it += mem_size;
        }
        return constant_value;
    }

    Parsing_status fully_parse() override {
        if (is_codegen_ready(status)) return status;
        for (auto& v : value) {
            if (is_error(v->fully_parse())) {
                status = v->status;
                return status;
            }
        }
        status = Parsing_status::FULLY_PARSED;
        return status;
    }

    Parsing_status finalize() override {
        if (is_codegen_ready(status)) return status;
        get_type();
        ASSERT(member_type);
        ASSERT(seq_type);
        for (const auto& v : value) {
            if (!is_codegen_ready(v->finalize())) {
                status = v->status;
                return status;
            }
            if (*v->get_type() != *member_type) {
                log_error("Sequence literal member type " + v->get_type()->toS() + " doesn't match the type of the sequence", v->context);
                add_note("Sequence starting here is of type " + seq_type->toS(), context);
                status == Parsing_status::TYPE_ERROR;
                return status;
            }
        }

        status = Parsing_status::FULLY_RESOLVED;
        return status;
    }

    void generate_code(std::ostream& target) override
    {
        ASSERT(is_codegen_ready(status));
        if (has_constant_value()) {
            seq_type->generate_literal(target, constant_value);
        } else {

        }
        // value.generate_literal(target);
        status = Parsing_status::CODE_GENERATED;
    }
};

