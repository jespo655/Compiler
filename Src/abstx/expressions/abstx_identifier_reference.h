
#include "variable_expression.h"
#include "abstx_identifier.h"
#include "../abstx_scope.h"

/*
An identifier reference is a variable expreesion, referencing a identifier defined somewhere else.
The reference is owned by the statement it is in.

This class is mostly here so the same identifier can be used and "owned" by several places, while the original identifier
  is only declared once (and is owned by that declaration statement).
*/

struct Abstx_identifier_reference : Variable_expression {
    std::string name = "";
    Shared<Abstx_identifier> id = nullptr;

    std::string toS() const override {
        if (id) return id->toS();
        return name;
    }

    virtual Shared<const CB_Type> get_type() override {
        if (id) return id->get_type();
        else return nullptr;
    }

    bool has_constant_value() const override {
        ASSERT(id);
        return id->has_constant_value();
    }

    const Any& get_constant_value() override {
        ASSERT(id);
        return id->get_constant_value();
    }

    void generate_code(std::ostream& target) const override {
        ASSERT(id);
        return id->generate_code(target);
    }

    void finalize() override {
        if (is_error(status) || is_codegen_ready(status)) return;
        if (id == nullptr) {
            ASSERT(name != ""); // must be set at construction
            id = parent_scope()->get_identifier(name);
        }
        if (id != nullptr) {
            id->finalize();
            status = id->status;
            std::cout << "Abstx_identifier_reference.finalize(): finalized identifier has status " << id->status << std::endl;
        } else {
            std::cout << "Abstx_identifier_reference.finalize(): failed to get identifier -> setting Parsing_status::DEPENDENCIES_NEEDED" << std::endl;
            status = Parsing_status::DEPENDENCIES_NEEDED;
        }
    }
};
