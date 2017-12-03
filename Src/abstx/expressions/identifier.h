#pragma once

#include "variable_expression.h"

#include <sstream>


struct Identifier : Variable_expression {
    shared<CB_Type> type = nullptr; // nullptr if not yet inferred
    CB_String name = "";
    CB_Any value; // For default value, use type.default_value()

    std::string toS() const override {
        ASSERT(name.size > 0);
        std::ostringstream oss;
        oss << name.toS() << ":";
        if (type == nullptr) oss << "???";
        else oss << type->toS();
        return oss.str();
    }

    virtual shared<CB_Type> get_type() { return type; }

    virtual seq<owned<Value_expression>> eval()
    {
        seq<owned<Value_expression>> s;
        s.add(owning_pointer_cast<Value_expression>(alloc<Identifier>(*this)));
        return s;
    }

};