#pragma once

#include "../abstx/abstx_scope.h"
// #include "../abstx/statements/abstx_function_call.h"
#include "token.h"
#include "token_iterator.h"

#include <string>

#include "../utilities/sequence.h"
#include "../utilities/pointers.h"



/*
Structure:


Step 1: read_scope, read_statement
    Starting with nothing but a token iterator, create an abstx node of some kind and return it
    These functions should do the absolute minimal work necessary
    In this step, types cannot be guaranteed to be resolved (never use get_type(), it might return nullptr)
    Important:
        Set owner, context (of first token) and start_token_index (where fully_parse should continue parsing)
        Set status to PARTIALLY_PARSED or error

    + Parsing_status read_statement(it, parent_scope): read a generic statement; add it to the parent scope and return parsing status



Step 2: Statement::fully_parse(), read_value_expression, read_variable_expression
    Starting with a PARTIALLY_PARSED statement, go through the list of tokens again and fill in any blanks
    (If status is not PARTIALLY_PARSED or DEPENDENCIES_NEEDED, do nothing)
    In this step, all types must be known. If get_type() returns nullptr, set status to TYPE_ERROR
    Important:
        Set status to FULLY_PARSED or error

    + Parsing_status Statement::fully_parse(): Starting with start_token_index set in read_statement(), continue and parse the statement, including value/variable expressions
    + Owned<Value_expression> read_value_expression(it, parent_scope): read a generic value expression and return an owned pointer to it
    + Owned<Variable_expression> read_variable_expression(it, parent_scope): read a generic variable expression and return an owned pointer to it

    For static scopes, statements can be resolved in any order. Keep a list of statements with DEPENDENCIES_NEEDED.
        If a statement cannot be resolved with DEPENDENCIES_NEEDED, add it to the list and try to resolve the next statement.
    For dynamic scopes, statments must be resolved in order. If parsing fails with DEPENDENCIES_NEEDED, return and try again from the beginning later


*/

// TODO: remove all finalize() from abstx
// TODO: remove all fully_parse() from non-statement abstx; move virtual function to Statement
// TODO: move all implementations of fully_parse() to statement_parser.cpp
// TODO: add function to resolve dependencies for statements that failed with DEPENDENCIES_NEEDED in fully_parse() (necessary for static scopes)


// TODO: for read_function_call(): check if its owner is Abstx_assignment; if it is, grab its identifiers as lhs. Remove LHS as an argument to the function






// parser.cpp
// Global scopes are allocated and stored internally - trying to parse a file that has already been parsed will just return a pointer to the old global scope
Shared<Global_scope> parse_file(const std::string& file_name);
Shared<Global_scope> parse_string(const std::string& string, const std::string& string_name, const Token_context& context);
Shared<Global_scope> parse_tokens(Seq<Token>&& tokens, const std::string& name);
Shared<Global_scope> read_global_scope(Seq<Token>&& tokens, const std::string& name);


// temporary variable names should be named _cb_tmp_uid, so we can be sure that they won't nameclash with other things

// statement_parser.cpp
// reading of statements: identifies which statement it is, and adds it to the parent scope
// for dynamic scopes, the statement has to be fully parsed and finalized before returning
// returns the status of the read statement
Parsing_status read_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope);

Parsing_status read_anonymous_scope(Token_iterator& it, Shared<Abstx_scope> parent_scope);
Parsing_status read_if_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope);
Parsing_status read_for_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope);
Parsing_status read_while_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope);
Parsing_status read_return_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope);
Parsing_status read_defer_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope);
Parsing_status read_using_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope);
Parsing_status read_c_code_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope);
Parsing_status read_declaration_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope);
Parsing_status read_assignment_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope);
Parsing_status read_value_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope);

// maybe should this also return only Parsing_status?
Shared<Abstx_function_call> read_run_expression(Token_iterator& it, Shared<Abstx_scope> parent_scope);

#define DEFAULT_OPERATOR_PRIO 1000
Owned<Value_expression> read_value_expression(Token_iterator& it, Shared<Abstx_scope> parent_scope, int min_operator_prio = DEFAULT_OPERATOR_PRIO);
Owned<Variable_expression> read_variable_expression(Token_iterator& it, Shared<Abstx_scope> parent_scope, int min_operator_prio = DEFAULT_OPERATOR_PRIO);

Owned<Value_expression> read_sequence_literal(Token_iterator& it, Shared<Abstx_scope> parent_scope);
Owned<Value_expression> read_simple_literal(Token_iterator& it, Shared<Abstx_scope> parent_scope);


/*

Pass 1:

Read static scopes:
    Declarations (up to ':' token; add identifiers to scope)
    Using statements (only "using" token; add statement to special list in scope)
    Chained static scopes (partially parse just like global scope)
    #run statements (only "#run" token; add statement to special list in GLOBAL scope)

Not allowed:
    Assignment (ends with ';')
    If, For, While (ends with dynamic scope '{}')
    Return (ends with ';')
    Statements with only function calls or operators (ends with ';')
    Other statements which cannot yet be identified (ends with ';')



Pass 2:

Compile statements and go through dynamic scopes:
All statements allowed
Compile immediately, using the current state of the local scopes

Start with a #run. When done, eval it. Log error if failed to FULLY_RESOLVE immediately.


Pass 3:

Just like pass 2, but instead if evaling compiled functions, output C code that represents it
Start at set entry point
When done, if no logged errors, construct a gcc call and compile the c code

*/










/*
Demo notes:



Vector :: struct (n: s64) {
    a : [N] s64;
}

foo := (v : $T/Vector) { ... }

Table :: struct (Key_type: Type, Value_type: Type) { ... }

foo := (table: *$T/Table, key: T.Key_type, value: T.Value_type) { ... }
foo := (table: *Table($K, $V), key: K, value: V) { ... }


KANSKE:
Struct definitions använder parent scope för att spara funktioner
Första argumentet är alltid *this
dot operator function calls kollar genom struct_type.parent_scope() efter matchande fn
Problem: Flera fner med samma namn i olika structar clashar




*/

/*
TODO:

rewrite av fn, op, struct, hur de sparas i scope

type system måste funka runtime -> ge varje type ett value (UID)
type comparison: uid==uid

typeof: endast compile time?

*/


