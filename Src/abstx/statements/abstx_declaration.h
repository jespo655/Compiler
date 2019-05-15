#pragma once

#include "abstx_statement.h"

/*
Syntax:
a : int = 1;    // declares a as the int 1
a := 1;         // infers the type from the value
a : int;        // infers the default value from the type
a := b;         // infers type from the variable b. The value of b overwrites the default value from the type of b.
a, b := foo();  // The lhs and rhs count will not match if a function in rhs returns more than one value.
*/

struct Abstx_identifier;
struct Value_expression;

struct Abstx_declaration : Statement {

    Seq<Owned<Abstx_identifier>> identifiers;
    Seq<Owned<Value_expression>> type_expressions;
    Seq<Owned<Value_expression>> value_expressions;

    std::string toS() const override;

    Parsing_status fully_parse() override;

    void generate_code(std::ostream& target, const Token_context& context) const override;

};

/*

a : int = 1;
a := 1;
a : int;
a := b;
a, b := foo();

// Generates c-code:

int a = 1;      // a : int = 1;         // trivial
int a = 1;      // a := 1;              // the type int is inferred from the literal 1
int a = 0;      // a : int;             // the value 0 is the default value of an int
T a = b;        // a := b;              // the type T is inferred from the identifier b
T1 a; T2 b; foo(&a,&b); // a, b := foo();


*/

