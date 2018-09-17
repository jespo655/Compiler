#pragma once

#include "abstx_statement.h"
#include "../abstx_scope.h"
#include "../expressions/value_expression.h"
#include "../../types/cb_primitives.h"

/*
while (b) {}
*/

struct Abstx_while : Statement {

    Owned<Value_expression> condition;
    Owned<Abstx_scope> scope;

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

    Parsing_status fully_parse() override; // implemented in statement_parser.cpp

    void generate_code(std::ostream& target) const override {
        ASSERT(is_codegen_ready(status));
        target << "while (";
        condition->generate_code(target);
        target << ") ";
        scope->generate_code(target);
    };

};


/*

// Generates c-code:

while (b) {}

*/
