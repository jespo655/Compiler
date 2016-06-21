#pragma once

#include "statement.h"

/*
defer a = 2; // performs the following statement at the end of scope
Consecutive defer statements in the same order are performed in reverse order at end of scope
Think of it like a "defer stack"
*/

struct Defer_statement : Statement {

    std::shared_ptr<Statement> statement;

    bool allow_in_static_scope() const override { return false; }
    bool allow_in_dynamic_scope() const override { return true; }

    std::string toS() const override {
        ASSERT(statement != nullptr);
        return "defer " + statement->toS();
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

