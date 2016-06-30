#pragma once

#include "variable_expression.h"

#include "literal.h"
#include "type.h"
#include "../utilities/assert.h"

#include <string>
#include <sstream>


struct Identifier : Variable_expression {
    std::shared_ptr<Type> type;
    std::string name = "";
    std::shared_ptr<Literal> default_value;

    std::string toS() const override {
        ASSERT(!name.empty());
        std::ostringstream oss;
        oss << name << ":";
        if (type == nullptr) oss << "???";
        else oss << type->toS();
        if (default_value != nullptr) oss << "=" << default_value->toS();
        return oss.str();
    }

    virtual std::shared_ptr<Type> get_type() { return type; }

};