#pragma once

#include "../abstx/abstx_scope.h"
// #include "../abstx/statements/abstx_function_call.h"
#include "token.h"
#include "token_iterator.h"

#include <string>

#include "../utilities/sequence.h"
#include "../utilities/pointers.h"

// parser.cpp
Shared<Global_scope> parse_file(const std::string& file_name);
Shared<Global_scope> parse_string(const std::string& string, const std::string& name = ""); // FIXME: add string context
Shared<Global_scope> parse_tokens(const Seq<Token>& tokens, const std::string& name = "");



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


