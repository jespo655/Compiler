#include "abstx_for.h"
#include "../abstx_scope.h"
#include "../expressions/value_expression.h"
#include "../../utilities/unique_id.h"
#include "../../types/cb_range.h" // CB_Iterable

std::string Abstx_for::toS() const { return "for(){}"; }

void Abstx_for::debug_print(Debug_os& os, bool recursive) const
{
    // FIXME: better for::toS()

    os << "For(";
    if (recursive) {
        ASSERT(range != nullptr);
        os << range->toS();
    }
    os << ") ";
    if (recursive) {
        ASSERT(scope != nullptr);
        scope->debug_print(os, recursive);
    }
    else os << std::endl;
}

void Abstx_for::generate_code(std::ostream& target, const Token_context& context) const {
    ASSERT(is_codegen_ready(status));

    if (anonymous_range) {
        // declare range
        target << "{ ";
        range->generate_code(target, context);
        target << " = ";
        range->value.generate_literal(target, context);
        target << "; ";
    }

    iterable_type->generate_for(target, range->name, it->name, step, reverse, anonymous_range);
    scope->generate_code(target, context);
    iterable_type->generate_for_after_scope(target);

    if (anonymous_range) {
        // close brace from before
        target << "}" << std::endl;
    }
}
