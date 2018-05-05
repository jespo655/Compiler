#pragma once

#include "variable_expression.h"
#include "../../types/cb_any.h"

#include <sstream>


struct Abstx_identifier : Variable_expression {
    std::string name = "";
    Any value; // might not have an actual value, but must have a type. Constant declared identifiers must have a value

    Shared<Value_expression> value_expression = nullptr; // pointer to the value expression that defined this identifier, or nullptr if not applicable

    std::string toS() const override {
        ASSERT(name.length() > 0);
        std::ostringstream oss;
        oss << name << ":";
        if (value.v_type == nullptr) oss << "???";
        else oss << value.v_type->toS();
        if (value.v_ptr != nullptr) oss << "=" << value.toS();
        return oss.str();
    }

    virtual Shared<const CB_Type> get_type() override {
        return value.v_type;
    }

    bool has_constant_value() const override {
        return value.v_ptr != nullptr || (value_expression != nullptr && value_expression->has_constant_value());
    }

    void const* get_constant_value() override {
        if (value.v_ptr != nullptr) return value.v_ptr;
        if (value_expression != nullptr && value_expression->has_constant_value()) value.v_ptr = value_expression->get_constant_value();
        return value.v_ptr;
    }

    Parsing_status fully_parse() override {
        return owner->fully_parse();
    }

    Parsing_status finalize() override {
        if (is_codegen_ready(status)) return status;
        ASSERT(name != ""); // should have been set immediately at first pass
        if (value.v_type == nullptr) status == Parsing_status::DEPENDENCIES_NEEDED; // should be set by the declaration statement's fully_parse()
        else status = Parsing_status::FULLY_RESOLVED; // no need to check for constant value - it will be set at the same time as the type (if it exists)
        return status;
    }

    void generate_code(std::ostream& target) override
    {
        // this should ouput the identifier used as a variable, since it's a subclass of Variable_expression
        ASSERT(name != "");
        ASSERT(is_codegen_ready(status));
        target << name;
        status = Parsing_status::CODE_GENERATED;
    }

};


struct Abstx_identifier_reference : Variable_expression {
    std::string name = "";
    Shared<Abstx_identifier> id = nullptr;

    std::string toS() const override {
        if (id) return id->toS();
        return name;
    }

    virtual Shared<const CB_Type> get_type() override {
        finalize();
        ASSERT(id);
        return id->get_type();
    }

    bool has_constant_value() const override {
        ASSERT(id);
        return id->has_constant_value();
    }

    void const* get_constant_value() override {
        ASSERT(id);
        return id->get_constant_value();
    }

    Parsing_status fully_parse() override {
        if (status != Parsing_status::PARTIALLY_PARSED) return status;
        ASSERT(name != "");
        status = Parsing_status::FULLY_PARSED;
        return status;
    }

    Parsing_status finalize() override;

    void generate_code(std::ostream& target) override {
        finalize();
        return id->generate_code(target);
    }
};