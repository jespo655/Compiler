#include "abstx_while.h"
#include "../../types/cb_primitives.h"

void Abstx_while::debug_print(Debug_os& os, bool recursive) const
{
    os << "while(";
    if (recursive) {
        ASSERT(condition != nullptr);
        os << condition->toS();
    }
    os << ") ";
    if (recursive) {
        ASSERT(scope != nullptr);
        scope->debug_print(os, recursive);
    }
    else os << std::endl;
}

void Abstx_while::generate_code(std::ostream& target, const Token_context& context) const {
    ASSERT(is_codegen_ready(status));
    target << "while (";
    condition->generate_code(target, context);
    target << ") ";
    scope->generate_code(target, context);
}

