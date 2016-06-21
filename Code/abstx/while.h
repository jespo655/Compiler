#pragma once

#include "statement.h"
#include "evaluated_value.h"
#include "scope.h"

/*
while (b) {}
*/

struct While_statement : Statement {

    std::shared_ptr<Evaluated_value> condition;
    std::shared_ptr<Scope> scope;

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