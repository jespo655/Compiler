#pragma once

#include "../abstx_node.h"

namespace Cube {

// A statement is a line of code.
// This is also the smallest unit of compilation.
struct Statement : Abstx_node
{
    // fully_parse(): finish a partial parse, from start_token_index.
    // This is done recursively, as an extension of read_X (see parser.h)
    // The parsing status is updated, then returned.
    // This function should only be called when the abstx node has finished parsing, and is expected to be complete.
    virtual Parsing_status fully_parse() = 0; // implemented in statement_parser.cpp
};


// An unknown statement is only used when an error has occured.
struct Unknown_statement : Statement
{
    std::string toS() const override { return "Unknown statement"; }
};

}
