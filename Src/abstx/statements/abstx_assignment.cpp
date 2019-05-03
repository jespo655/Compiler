#include "abstx_assignment.h"
#include "../expressions/variable_expression.h"
#include "../expressions/value_expression.h"

namespace Cube {


std::string Abstx_assignment::toS() const {
    ASSERT(lhs.size > 0);
    ASSERT(rhs.size > 0);

    std::ostringstream oss;
    bool first = true;
    for (auto& var : lhs) {
        ASSERT(var != nullptr);
        if (!first) oss << ", ";
        oss << var->toS();
        first = false;
    }
    oss << " = ";
    first = true;
    for (auto& val : rhs) {
        ASSERT(val != nullptr);
        if (!first) oss << ", ";
        oss << val->toS();
        first = false;
    }
    oss << ";";
    return oss.str();
}


void Abstx_assignment::generate_code(std::ostream& target) const {
    ASSERT(is_codegen_ready(status));
    ASSERT(lhs.size == rhs.size);
    for (int i = 0; i < lhs.size; ++i) {
        lhs[i]->generate_code(target); // this should be a valid c style lvalue
        target << " = ";
        if (rhs.size == 1) rhs[0]->generate_code(target);
        else rhs[i]->generate_code(target);
        target << ";" << std::endl;
    }
};

}
