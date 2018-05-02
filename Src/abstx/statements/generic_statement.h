#pragma once

#include "abstx_statement.h"

/*
Possible solution for generic functions:
A generic statement wants a type map from generic type to real type
When resolved, the parser supplies that type map.
The statement can be resolved several times with different type maps.
Results are stored in a map of generated statements, with the type map as the key.

NYI
*/

// struct Generic_statement : Statement
// {
//     Owned<Statement> statement;

//     std::string toS() const override { ASSERT(statement != nullptr); return "generic "+statement->toS(); }
// }
