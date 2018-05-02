#pragma once

#include "variable_expression.h"
#include "../../types/cb_any.h"

#include <sstream>


struct Identifier : Variable_expression {
    shared<const CB_Type> cb_type = nullptr; // nullptr if not yet inferred
    std::string name = "";
    // any value; // For default value, use cb_type.default_value() (@todo should this even be here?)

    std::string toS() const override {
        ASSERT(name.length() > 0);
        std::ostringstream oss;
        oss << name << ":";
        if (cb_type == nullptr) oss << "???";
        else oss << cb_type->toS();
        return oss.str();
    }

    virtual shared<const CB_Type> get_type() override {
        return cb_type;
    }

    Parsing_status finalize() override {
        if (is_codegen_ready(status)) return status;
        ASSERT(name != "");
        // we reached the end -> we are done
        status = Parsing_status::FULLY_RESOLVED;
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
