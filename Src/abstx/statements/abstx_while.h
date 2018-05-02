#pragma once

#include "abstx_statement.h"
#include "abstx_scope.h"
#include "../expressions/value_expression.h"

/*
while (b) {}
*/

struct While_statement : Statement {

    owned<Value_expression> condition;
    owned<CB_Scope> scope;

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