#pragma once

#include "abstx.h"
#include "value_expression.h"

// A statement is a line of code.
// This is also the smallest unit of compilation.
struct Statement : Abstx_node
{

};


// An unknown statement is only used when an error has occured.
struct Unknown_statement : Statement
{
    std::string toS() const override { return "Unknown statement"; }
};


// An expression statement is a single expression on a line.
//   Some examples:
// foo();
// ++i;
struct Expression_statement : Statement
{
    std::shared_ptr<Value_expression> expr;
    std::string toS() const override { return "Expression statement"; }
};

