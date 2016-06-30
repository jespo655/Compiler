#pragma once

#include "statement.h"
#include "value_expression.h"
#include "scope.h"

/*
while (b) {}
*/

struct While_statement : Statement {

    std::shared_ptr<Value_expression> condition;
    std::shared_ptr<Scope> scope;

    bool allow_in_static_scope() const override { return false; }
    bool allow_in_dynamic_scope() const override { return true; }

    std::string toS() const override { return "while(){}"; }

    void debug_print(Debug_os& os, bool recursive=true) const override
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

};


/*

// Generates c-code:

while (b) {}

*/