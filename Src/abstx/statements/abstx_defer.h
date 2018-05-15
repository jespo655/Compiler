#pragma once

#include "abstx_statement.h"

/*
defer a = 2; // performs the following statement at the end of scope
Consecutive defer statements in the same order are performed in reverse order at end of scope
Think of it like a "defer stack"
*/

struct Abstx_defer : Statement {

    Owned<Statement> statement;

    std::string toS() const override {
        ASSERT(statement != nullptr);
        return "defer " + statement->toS();
    }

    Parsing_status fully_parse() override; // implemented in statement_parser.cpp

    void generate_code(std::ostream& target) const override {
        ASSERT(is_codegen_ready(status));
        statement->generate_code(target);
    }
};


/*

{
    a := 1;
    defer a = 2;
    defer a = 3;
    a = 4;
}

// Generates c-code:

{
    int a;
    a = 1;
    a = 4;
    a = 3; // deferred last
    a = 2; // deferred first
}

*/

