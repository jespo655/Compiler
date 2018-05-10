#pragma once

#include "variable_expression.h"
#include "../../types/cb_any.h"
#include "../../utilities/unique_id.h"

#include <sstream>


struct Abstx_identifier : Variable_expression {
    std::string name = "";
    Any value; // might not have an actual value, but must have a type. Constant declared identifiers must have a value
    uint64_t uid = get_unique_id(); // set to 0 to avoid using suffixes

    // pointer to the value expression that defined this identifier, or nullptr if not applicable
    // This can be non-standard CB values, for example be a function or scope expression
    Shared<Value_expression> value_expression = nullptr;

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
        if (value.v_type == nullptr && value_expression != nullptr) value.v_type == value_expression->get_type();
        return value.v_type;
    }

    bool has_constant_value() const override {
        return value.v_ptr != nullptr || (value_expression != nullptr && value_expression->has_constant_value());
    }

    const Any& get_constant_value() override {
        if (value.v_ptr != nullptr) return value;
        if (value_expression != nullptr && value_expression->has_constant_value()) value.v_ptr = value_expression->get_constant_value().v_ptr;
        return value;
    }

    void generate_code(std::ostream& target) override
    {
        // this should ouput the identifier used as a variable, since it's a subclass of Variable_expression
        ASSERT(name != "");
        ASSERT(is_codegen_ready(status));
        target << name;
        if (uid) target << "_" << uid; // uid suffix to avoid name C name clashes
        status = Parsing_status::CODE_GENERATED;
    }

    void finalize() {
        if (name == "") return;
        if (value.v_type == nullptr) return;
        status = Parsing_status::FULLY_RESOLVED;
    }

};
