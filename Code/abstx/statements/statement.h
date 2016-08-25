#pragma once

#include "../abstx.h"

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


/*
Statement:
    If
    For
    While
    Return
    Assignment
    Declaration
    Using
    Anonymous scope
    Pure value_expression / function call (operators or function call with side effects, just a single identifier is not ok)
    Defer
Modifiers: Generic
*/