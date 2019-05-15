#include "abstx_if.h"
#include "../../types/cb_primitives.h"

std::string Abstx_if::Abstx_conditional_scope::toS() const { return "if(){}"; }

void Abstx_if::Abstx_conditional_scope::debug_print(Debug_os& os, bool recursive) const
{
    os << "if(";
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

void Abstx_if::Abstx_conditional_scope::generate_code(std::ostream& target, const Token_context& context) const {
    ASSERT(is_codegen_ready(status));
    target << "if (";
    condition->generate_code(target, context);
    target << ") ";
    scope->generate_code(target, context);
}

Parsing_status Abstx_if::Abstx_conditional_scope::fully_parse() {
    if (is_codegen_ready(status) || is_error(status)) return status;
    condition->finalize();
    scope->fully_parse();

    if (is_error(condition->status) && !is_fatal(status)) status = condition->status;
    if (is_error(scope->status) && !is_fatal(status)) status = scope->status;
    if (!is_error(status)) status = Parsing_status::FULLY_RESOLVED;
    return status;
}



std::string Abstx_if::toS() const
{
    std::ostringstream oss;
    bool first = true;
    for (auto& cs : conditional_scopes) {
        ASSERT(cs != nullptr);
        if (!first) oss << " els";
        oss << cs->toS();
        first = false;
    }
    if (else_scope != nullptr) oss << " else{}";
    // if (then_scope != nullptr) oss << " then{}";
    return oss.str();
}

void Abstx_if::debug_print(Debug_os& os, bool recursive) const
{
    for (auto& cs : conditional_scopes) {
        ASSERT(cs != nullptr);
        cs->debug_print(os, recursive);
    }
    if (else_scope != nullptr) {
        os << "else ";
        if (recursive) else_scope->debug_print(os, recursive);
    }
    // if (then_scope != nullptr) {
    //     os << "then ";
    //     if (recursive) then_scope->debug_print(os, recursive);
    // }
}

void Abstx_if::generate_code(std::ostream& target, const Token_context& context) const {
    if (!is_codegen_ready(status)) LOG("status is " << status);
    ASSERT(is_codegen_ready(status));
    for (int i = 0; i < conditional_scopes.size; ++i) {
        if (i) target << "else ";
        conditional_scopes[i]->generate_code(target, context);
    }
    if (else_scope != nullptr) {
        target << "else ";
        else_scope->generate_code(target, context);
    }
    // @todo: add support for then-scopes (needs support for adding a statement to a scope) see syntax below
}

