#pragma once

#include "abstx_statement.h"
#include "../abstx_scope.h"
#include "../expressions/value_expression.h"

/*
while (b) {}
*/

struct Abstx_while : Statement {

    Owned<Value_expression> condition;
    Owned<Abstx_scope> scope;

    std::string toS() const override { return "while(){}"; }

    void debug_print(Debug_os& os, bool recursive=true) const override;
    Parsing_status fully_parse() override;
    void generate_code(std::ostream& target) const override;

};

/*

// Generates c-code:

while (b) {}

*/
