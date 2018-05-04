#pragma once

#include "variable_expression.h"
#include "../../types/cb_any.h"

#include <sstream>


struct Abstx_identifier : Variable_expression {
    std::string name = "";
    Any value; // might not have an actual value, but must have a type. Constant declared identifiers must have a value

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

    bool has_constant_value() const {
        return value.v_ptr != nullptr;
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
