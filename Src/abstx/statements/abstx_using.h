#pragma once

#include "abstx_statement.h"
#include "../expressions/value_expression.h"

/*
Synatx:
s := {}; // s is assigned a scope literal
using s; // all names from s are pulled into the local scope

struct ST{};    // ST is a struct type
s := ST();      // s is an instance of the struct ST
using s;        // all members of s is pulled into the local scope



MAYBE:
using a:=s;     // the members of a is pulled in, but are also accessable through a
using a:=ST();  // the members of a is pulled in, but are also accessable through a
    // this is using the regular declaration syntax

TODO: decide exact syntax and usage, before implementing the class

*/

struct Using_statement : Statement {

    Owned<Value_expression> subject;

    std::string toS() const override { return "using statement"; }
};


/*


// Generates c-code:


*/

