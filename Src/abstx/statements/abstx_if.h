#pragma once

#include "abstx_statement.h"
#include "../abstx_scope.h"
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




struct Abstx_if : Statement {

    struct Abstx_conditional_scope : Abstx_node {

        Owned<Value_expression> condition;
        Owned<Abstx_scope> scope;

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

        void generate_code(std::ostream& target) const override {
            ASSERT(is_codegen_ready(status));
            target << "if (";
            condition->generate_code(target);
            target << ") ";
            scope->generate_code(target);
        };
    };


    Seq<Owned<Abstx_conditional_scope>> conditional_scopes;
    Owned<Abstx_scope> else_scope; // is entered if none of the conditional scopes are entered
    // Owned<Abstx_scope> then_scope; // is entered if not the else_scope is entered // @todo: find a good way to express this in C code

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

    Parsing_status fully_parse() override; // implemented in statement_parser.cpp

    void generate_code(std::ostream& target) const override {
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
