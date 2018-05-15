#pragma once

#include "variable_expression.h"
#include "value_expression.h"
#include "abstx_identifier.h"
#include "../../types/cb_pointer.h"

struct Abstx_pointer_dereference : Variable_expression {

    Shared<Abstx_identifier> pointer_id; // Owned by parent scope

    std::string toS() const override {
        ASSERT(pointer_id->name.length() > 0);
        std::ostringstream oss;
        oss << "*" << pointer_id->name;
        return oss.str();
    }

    virtual Shared<const CB_Type> get_type() override {
        ASSERT(pointer_id);
        Shared<const CB_Pointer> type = dynamic_pointer_cast<const CB_Pointer>(pointer_id->get_type());
        ASSERT(type != nullptr);
        ASSERT(type->v_type != nullptr);
        return type->v_type;
    }

    void generate_code(std::ostream& target) const override
    {
        ASSERT(is_codegen_ready(status));
        target << "*";
        pointer_id->generate_code(target);
    }

};


struct Abstx_address_of : Value_expression {

    Shared<Abstx_identifier> pointer_id; // Owned by parent scope
    Shared<const CB_Type> pointer_type = nullptr;

    std::string toS() const override {
        ASSERT(pointer_id->name.length() > 0);
        std::ostringstream oss;
        oss << "*" << pointer_id->name;
        return oss.str();
    }

    virtual Shared<const CB_Type> get_type() override {
        ASSERT(pointer_id);
        if (pointer_type == nullptr) {
            Owned<CB_Pointer> pt = alloc(CB_Pointer());
            pt->v_type = pointer_id->get_type();
            pt->finalize();
            pointer_type = add_complex_cb_type(owned_static_cast<CB_Type>(std::move(pt)));
        }
        return pointer_type;
    }

    void generate_code(std::ostream& target) const override
    {
        ASSERT(is_codegen_ready(status));
        target << "&";
        pointer_id->generate_code(target);
    }

};
