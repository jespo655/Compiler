#include "abstx_declaration.h"
#include "../expressions/abstx_identifier.h"
#include "../expressions/value_expression.h"

std::string Abstx_declaration::toS() const {
    ASSERT(identifiers.size > 0);

    std::ostringstream oss;
    bool first = true;
    bool all_typed = true;
    for (auto& id : identifiers) {
        ASSERT(id != nullptr);
        ASSERT(id->name.length() > 0);
        if (!first) oss << ", ";
        oss << id->name;
        first = false;
        if (id->get_type() == nullptr) all_typed = false;
    }
    if (all_typed) {
        oss << " : ";
        first = true;
        for (auto& id : identifiers) {
            ASSERT(id != nullptr);
            ASSERT(id->get_type() != nullptr);
            if (!first) oss << ", ";
            oss << id->get_type()->toS();
            first = false;
        }
    } else {
        ASSERT(value_expressions.size > 0, context.toS());
    }
    if (value_expressions.size > 0) {
        if (all_typed) oss << " = ";
        else oss << " := ";
        first = true;
        for (auto& ev : value_expressions) {
            ASSERT(ev != nullptr);
            if (!first) oss << ", ";
            oss << ev->toS();
            first = false;
        }
    }
    oss << ";";
    return oss.str();
}

void Abstx_declaration::generate_code(std::ostream& target) const {
    ASSERT(is_codegen_ready(status), "something went wrong in declaration "+toS());
    if (value_expressions.empty()) {
        // explicit uninitialized
        for (int i = 0; i < identifiers.size; ++i) {
            ASSERT(identifiers[i]); // can't be nullpointer
            identifiers[i]->get_type()->generate_type(target);
            target << " ";
            identifiers[i]->generate_code(target); // this should be a variable name
            target << ";" << std::endl;
        }
    } else {
        ASSERT(value_expressions.size == 1 || value_expressions.size == identifiers.size);
        for (int i = 0; i < identifiers.size; ++i) {
            ASSERT(identifiers[i]); // can't be nullpointer
            identifiers[i]->get_type()->generate_type(target);
            target << " ";
            identifiers[i]->generate_code(target); // this should be a valid c style lvalue
            target << " = ";
            value_expressions[(value_expressions.size==1?0:i)]->generate_code(target); // this should be a valid c style lvalue
            target << ";" << std::endl;
        }
    }
};
