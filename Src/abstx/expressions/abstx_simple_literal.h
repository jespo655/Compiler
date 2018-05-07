#pragma once

#include "value_expression.h"
#include "../../types/cb_any.h"

#include <sstream>

struct Abstx_simple_literal : Value_expression {
    Any value; // has to have a value

    std::string toS() const override {
        std::ostringstream oss;
        oss << "(" << value.v_type->toS() << ")" << value.toS();
        return oss.str();
    }

    Shared<const CB_Type> get_type() override {
        return value.v_type;
    }

    bool has_constant_value() const {
        ASSERT(value.v_ptr != nullptr); // it should be set during creation
        return true;
    }

    void const* get_constant_value() override {
        return value.v_ptr;
    }

    void generate_code(std::ostream& target) override
    {
        ASSERT(is_codegen_ready(status));
        value.generate_literal(target);
        status = Parsing_status::CODE_GENERATED;
    }
};

