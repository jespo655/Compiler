#pragma once

#include "abstx_statement.h"
#include "abstx_scope.h"
#include "../expressions/value_expression.h"
#include "../../types/cb_primitives.h"

#include <sstream>

/*
Syntax:
if (b1) {}
elsif (b2) {}
elsif (b3) {}
else {}
then {}
*/

struct Conditional_scope : Abstx_node {

    owned<Value_expression> condition;
    owned<CB_Scope> scope;

    std::string toS() const override { return "if(){}"; }

    void debug_print(Debug_os& os, bool recursive=true) const override
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

    Parsing_status finalize() override {
        if (is_codegen_ready(status)) return status;

        if (!is_codegen_ready(condition->finalize())) {
            status = condition->status;
            return status;
        }
        seq<shared<const CB_Type>> types = condition->get_type();
        if (types.size != 1 || *types[0] != *CB_Bool::type) {
            status = Parsing_status::TYPE_ERROR;
            return status;
        }
        if (!is_codegen_ready(scope->finalize())) {
            status = scope->status;
            return status;
        }
        // we reached the end -> we are done
        status = Parsing_status::FULLY_RESOLVED;
        return status;
    }

    void generate_code(std::ostream& target) override {
        ASSERT(is_codegen_ready(status));
        target << "if (";
        condition->generate_code(target);
        target << ") ";
        scope->generate_code(target);
        status = Parsing_status::CODE_GENERATED;
    };

};




struct If_statement : Statement {

    seq<owned<Conditional_scope>> conditional_scopes;
    owned<CB_Scope> else_scope; // is entered if none of the conditional scopes are entered
    // owned<CB_Scope> then_scope; // is entered if not the else_scope is entered

    std::string toS() const override
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

    void debug_print(Debug_os& os, bool recursive=true) const override
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

    Parsing_status finalize() override {
        if (is_codegen_ready(status)) return status;
        for (const auto& cs : conditional_scopes) {
            if (!is_codegen_ready(cs->finalize())) {
                status = cs->status;
                return status;
            }
        }
        if (else_scope != nullptr && !is_codegen_ready(else_scope->finalize())) {
            status = else_scope->status;
            return status;
        }
        // if (then_scope != nullptr && !is_codegen_ready(then_scope->finalize())) {
        //     status = then_scope->status;
        //     return status;
        // }
        // we reached the end -> we are done
        status = Parsing_status::FULLY_RESOLVED;
        return status;
    }

    void generate_code(std::ostream& target) override {
        ASSERT(is_codegen_ready(status));
        for (int i = 0; i < conditional_scopes.size; ++i) {
            if (i) target << "else ";
            conditional_scopes[i]->generate_code(target);
        }
        if (else_scope != nullptr) {
            target << "else ";
            else_scope->generate_code(target);
        }
        // @todo: add support for then-scopes (needs support for adding a statement to a scope) see syntax below
        status = Parsing_status::CODE_GENERATED;
    };

};


/*

// Generates c-code:

bool __then_N = false;
if (b1) {}
else if (b2) {}
else { __then_N = true; } // else_scope with an extra assignment inserted (unless then_scope is null)
if (__then_N) {} // then_scope

*/
