#pragma once

#include "variable_expression.h"
#include "value_expression.h"
#include "abstx_identifier.h"
#include "../../types/cb_pointer.h"

struct Abstx_pointer_dereference : Variable_expression {

    Shared<Identifier> pointer_id; // Owned by parent scope

    std::string toS() const override {
        ASSERT(pointer_id->name.length() > 0);
        std::ostringstream oss;
        oss << "*" << pointer_id->name;
        return oss.str();
    }

    virtual Shared<const CB_Type> get_type() override {
        Shared<const CB_Pointer> type = dynamic_pointer_cast<const CB_Pointer>(pointer_id->get_type());
        ASSERT(type != nullptr);
        ASSERT(type->v_type != nullptr);
        return type->v_type;
    }

    Parsing_status finalize() override {
        if (is_codegen_ready(status)) return status;

        if (!is_codegen_ready(pointer_id->status)) {
            if (is_error(pointer_id->status)) status = pointer_id->status;
            else status = Parsing_status::DEPENDENCIES_NEEDED;
            return status;
        }
        // we reached the end -> we are done
        status = Parsing_status::FULLY_RESOLVED;
        return status;
    }

    void generate_code(std::ostream& target) override
    {
        ASSERT(is_codegen_ready(status));
        target << "*";
        pointer_id->generate_code(target);
        status = Parsing_status::CODE_GENERATED;
    }

};


struct Abstx_address_of : Value_expression {

    Shared<Identifier> pointer_id; // Owned by parent scope
    Owned<CB_Pointer> pointer_type = nullptr;

    std::string toS() const override {
        ASSERT(pointer_id->name.length() > 0);
        std::ostringstream oss;
        oss << "*" << pointer_id->name;
        return oss.str();
    }

    virtual Shared<const CB_Type> get_type() override {
        return static_pointer_cast<const CB_Type>(pointer_type);
    }

    Parsing_status finalize() override {
        if (is_codegen_ready(status)) return status;

        if (!is_codegen_ready(pointer_id->status)) {
            if (is_error(pointer_id->status)) status = pointer_id->status;
            else status = Parsing_status::DEPENDENCIES_NEEDED;
            return status;
        }

        if (pointer_type == nullptr) {
            pointer_type = alloc(CB_Pointer());
            pointer_type->v_type = pointer_id->get_type();
            pointer_type->finalize();
        }
        // we reached the end -> we are done
        status = Parsing_status::FULLY_RESOLVED;
        return status;
    }

    void generate_code(std::ostream& target) override
    {
        ASSERT(is_codegen_ready(status));
        target << "&";
        pointer_id->generate_code(target);
        status = Parsing_status::CODE_GENERATED;
    }

};
