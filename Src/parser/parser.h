#pragma once

#include "../abstx/statements/scope.h"
#include "../abstx/statements/function_statement.h"
#include "token.h"
#include "token_iterator.h"

#include <memory>
#include <string>
#include <vector>

typedef Abstx_scope Scope;

struct Global_scope : Scope
{
    std::string file_name;
    const std::vector<Token> tokens; // should be treated as const

    std::vector<std::shared_ptr<Function_call_statement>> run_statements;
    std::vector<std::shared_ptr<Unknown_statement>> unknown_statements;

    Global_scope(const std::vector<Token>& tokens) : tokens{tokens} {}

    Token_iterator iterator(int index=0) { return Token_iterator(tokens, index); }
};

// parser.cpp
std::shared_ptr<Global_scope> parse_file(const std::string& file_name);
std::shared_ptr<Global_scope> parse_string(const std::string& string, const std::string& name = ""); // FIXME: add string context
std::shared_ptr<Global_scope> parse_tokens(const std::vector<Token>& tokens, const std::string& name = "");

template<typename T>
Token_iterator get_iterator(std::shared_ptr<T> abstx, int index=0); // FIXME: undefined reference to implementation ?
std::shared_ptr<Global_scope> get_global_scope(std::shared_ptr<Scope> scope);


// FIXME: separate read_X(it&, parent) and compile_X(it&, parent)
// read_X would only be used in static contexts,
//    and would only do as little as possible (returns an abstx node and places it at the beginning of the next thing)
// compile_X would be used in dynamic contexts,
//    where everything should be compiled immediately.
//    It should immediately try to fully resolve the statement.
// natural extention: compile_X(string, parent, string_context)
//    just be careful with assertions, stuff like parens and eof is not checked if no global parsing pass is done first


// default compile_X behaviour:
// x = read_X(); fully_resolve_X(x); return x;
// more complex / optimized:
// return compile_X();


// scope_parser.cpp
std::shared_ptr<Global_scope> read_global_scope(const std::vector<Token>& tokens, const std::string& name);
std::shared_ptr<Scope> read_static_scope(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<Anonymous_scope> read_anonymous_static_scope(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<Scope> compile_dynamic_scope(Token_iterator& it, std::shared_ptr<Scope> parent_scope);


// identifier_parser.cpp
struct Identifier;
std::shared_ptr<Identifier> read_identifier(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<Identifier> compile_identifier(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
Parsing_status fully_resolve_identifier(std::shared_ptr<Identifier>& identifier);


// statement_parser.cpp
struct Statement;
std::shared_ptr<Statement> read_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<Statement> compile_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
// TODO:
Parsing_status fully_resolve_statement(std::shared_ptr<Statement> statement);


// using_parser.cpp
struct Using_statement;
std::shared_ptr<Using_statement> read_using_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
Parsing_status fully_resolve_using(std::shared_ptr<Using_statement> using_statement);
void resolve_imports(std::shared_ptr<Scope> scope);


// literal_parser.cpp
struct Literal;
std::shared_ptr<Literal> compile_int_literal(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<Literal> compile_float_literal(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<Literal> compile_bool_literal(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<Literal> compile_string_literal(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<Value_expression> compile_sequence_literal(Token_iterator& it, std::shared_ptr<Scope> parent_scope);


// declaration_parser.cpp
struct Declaration_statement;
std::shared_ptr<Declaration_statement> read_declaration_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
// TODO:
Parsing_status fully_resolve_declaration(std::shared_ptr<Declaration_statement>& declaration);
std::shared_ptr<Declaration_statement> compile_declaration_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);


// if_parser.cpp
struct If_statement;
std::shared_ptr<If_statement> read_if_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<If_statement> compile_if_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);


// for_parser.cpp
struct For_statement;
std::shared_ptr<For_statement> read_for_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
// TODO:
std::shared_ptr<For_statement> compile_for_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);


// while_parser.cpp
struct While_statement;
std::shared_ptr<While_statement> read_while_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
// TODO:
std::shared_ptr<While_statement> compile_while_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);


// return_parser.cpp
struct Return_statement;
std::shared_ptr<Return_statement> read_return_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
// TODO:
std::shared_ptr<Return_statement> compile_return_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);


// assignment_parser.cpp
struct Assignment_statement;
std::shared_ptr<Assignment_statement> read_assignment_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
// TODO:
std::shared_ptr<Assignment_statement> compile_assignment_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);


// defer_parser.cpp
struct Defer_statement;
std::shared_ptr<Defer_statement> read_defer_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<Defer_statement> compile_defer_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);


// expression_parser.cpp
struct Value_expression;
std::shared_ptr<Value_expression> compile_value_expression(Token_iterator& it, std::shared_ptr<Scope> parent_scope, int min_operator_prio=0);
std::shared_ptr<Variable_expression> compile_variable_expression(Token_iterator& it, std::shared_ptr<Scope> parent_scope);



// TODO:
// operators rewrite


// TODO:
struct Function_call;
struct Seq_lookup;
struct Getter;
std::shared_ptr<Value_expression> compile_function_call(Token_iterator& it, std::shared_ptr<Value_expression> fn_id);
std::shared_ptr<Value_expression> compile_seq_indexing(Token_iterator& it, std::shared_ptr<Value_expression> seq_id);
std::shared_ptr<Value_expression> compile_getter(Token_iterator& it, std::shared_ptr<Value_expression> struct_id);
std::shared_ptr<Statement> read_operator_declaration(Token_iterator& it, std::shared_ptr<Value_expression> struct_id);



// TODO:
// variable_expression_parser.cpp + helpers
// value_expression_parser.cpp + helpers
// (read_comma_separated_value_list?)






/*

Pass 1:

Read static scopes:
    Declarations (up to ':' token, add identifiers to scope)
    Using statements (only "using" token, add statement to special list in scope)
    Chained static scopes (partially parse just like global scope)
    #run statements (only "#run" token, add statement to special list in GLLOBAL scope)
    Infix_operator op := (add as operator to scope)
    Prefix_operator op := (add as operator to scope)

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


