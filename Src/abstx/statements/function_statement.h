#pragma once

#include "statement.h"
#include "../expressions/value_expression.h"

/*
Syntax:
    foo(); // all return values are ignored
    increment i; // increment is a prefix operator which modifies i
    i add 2; // add is an infix operator which modifies i
*/

struct Function_call_statement : Statement
{
    owned<Value_expression> expr;

    std::string toS() const override { return "function call statement"; }
}