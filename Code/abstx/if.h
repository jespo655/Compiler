#pragma once

#include "statement.h"
#include "evaluated_value.h"
#include "scope.h"

#include <vector>
#include <sstream>

/*
if (b1) {}
elsif (b2) {}
elsif (b3) {}
else {}
then {}
*/

struct Conditional_scope : Abstx_node {

    std::shared_ptr<Evaluated_value> condition;
    std::shared_ptr<Scope> scope;

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

};




struct If_statement : Statement {

    std::vector<std::shared_ptr<Conditional_scope>> conditional_scopes;
    std::shared_ptr<Scope> else_scope; // is entered if none of the conditional scopes are entered
    std::shared_ptr<Scope> then_scope; // is entered if not the else_scope is entered

    std::string toS() const override
    {
        std::ostringstream oss;
        bool first = true;
        for (auto cs : conditional_scopes) {
            ASSERT(cs != nullptr);
            if (!first) oss << " els";
            oss << cs->toS();
            first = false;
        }
        if (else_scope != nullptr) oss << " else{}";
        if (then_scope != nullptr) oss << " then{}";
        return oss.str();
    }

    void debug_print(Debug_os& os, bool recursive=true) const override
    {
        for (auto cs : conditional_scopes) {
            ASSERT(cs != nullptr);
            cs->debug_print(os, recursive);
        }
        if (else_scope != nullptr) {
            os << "else ";
            if (recursive) else_scope->debug_print(os, recursive);
        }
        if (then_scope != nullptr) {
            os << "then ";
            if (recursive) then_scope->debug_print(os, recursive);
        }
    }

};


/*

// Generates c-code:

bool __then_N = false;
if (b1) {}
else if (b2) {}
else { __then_N = true; } // else_scope with an extra assignment inserted (unless then_scope is null)
if (__then_N) {} // then_scope

*/