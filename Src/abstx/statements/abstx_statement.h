#pragma once

#include "../abstx.h"

// A statement is a line of code.
// This is also the smallest unit of compilation.
struct Statement : Abstx_node
{
    // fully_parse(): finish a partial parse, from start_token_index.
    // This is done recursively, as an extension of read_X (see parser.h)
    // The parsing status is updated, then returned.
    // This function should only be called when the abstx node has finished parsing, and is expected to be complete.
    virtual Parsing_status fully_parse() = 0; // implemented in statement_parser.cpp

    // make basic assertions and get the correct token iterator; to be called in the beginning of fully_parse()
    virtual Token_iterator parse_begin() const;
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
