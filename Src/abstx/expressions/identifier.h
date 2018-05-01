#pragma once

#include "variable_expression.h"
#include "../../types/cb_any.h"

#include <sstream>


struct Identifier : Variable_expression {
    shared<const CB_Type> cb_type = nullptr; // nullptr if not yet inferred
    std::string name = "";
    any value; // For default value, use cb_type.default_value()

    std::string toS() const override {
        ASSERT(name.length() > 0);
        std::ostringstream oss;
        oss << name << ":";
        if (cb_type == nullptr) oss << "???";
        else oss << cb_type->toS();
        return oss.str();
    }

    virtual seq<shared<const CB_Type>> get_type() override {
        seq<shared<const CB_Type>> s;
        s.add(cb_type);
        return s;
    }

    virtual seq<owned<Value_expression>> eval()
    {
        seq<owned<Value_expression>> s;
        s.add(owning_pointer_cast<Value_expression>(alloc<Identifier>(*this)));
        return s;
    }

    void generate_code(std::ostream& target) override
    {
        // @todo: what should be outputted here?
        // declaration with its value? just the identifier name?
    }

};