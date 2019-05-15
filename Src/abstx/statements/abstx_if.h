#pragma once

#include "abstx_statement.h"
#include "../expressions/value_expression.h"
#include "../abstx_scope.h"

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

        std::string toS() const override;

        void debug_print(Debug_os& os, bool recursive=true) const override;

        void generate_code(std::ostream& target, const Token_context& context) const override;

        Parsing_status fully_parse();
    };

    Seq<Owned<Abstx_conditional_scope>> conditional_scopes;
    Owned<Abstx_scope> else_scope; // is entered if none of the conditional scopes are entered
    // Owned<Abstx_scope> then_scope; // is entered if not the else_scope is entered // @todo: find a good way to express this in C code

    std::string toS() const override;

    void debug_print(Debug_os& os, bool recursive=true) const override;
    Parsing_status fully_parse() override;

    void generate_code(std::ostream& target, const Token_context& context) const override;

};

/*

// Generates c-code:

bool __then_N = false;
if (b1) {}
else if (b2) {}
else { __then_N = true; } // else_scope with an extra assignment inserted (unless then_scope is null)
if (__then_N) {} // then_scope

*/
