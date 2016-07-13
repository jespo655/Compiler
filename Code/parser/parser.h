#pragma once

#include "../abstx/scope.h"
#include "../abstx/function.h"
#include "token.h"

#include <memory>
#include <string>
#include <vector>



struct Global_scope : Scope
{
    std::string file_name;
    const std::vector<Token> tokens; // should be treated as const

    std::vector<std::shared_ptr<Function_call_statement>> run_statements;

    Global_scope(const std::vector<Token>& tokens) : tokens{tokens} {}

    Token_iterator iterator(int index=0) { return Token_iterator(tokens, index); }
};

// parser.cpp
std::shared_ptr<Global_scope> parse_file(const std::string& file_name);
std::shared_ptr<Global_scope> parse_string(const std::string& string, const std::string& name = ""); // FIXME: add string context
std::shared_ptr<Global_scope> parse_tokens(const std::vector<Token>& tokens, const std::string& name = "");

template<typename T>
Token_iterator get_iterator(std::shared_ptr<T> abstx, int index=0);
std::shared_ptr<Global_scope> get_global_scope(std::shared_ptr<Scope> scope);


// scope_parser.cpp
std::shared_ptr<Scope> read_static_scope(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<Scope> read_dynamic_scope(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<Scope> fully_resolve_dynamic_scope(std::shared_ptr<Scope> scope);




// identifier_parser.cpp
struct Identifier;
std::shared_ptr<Identifier> read_identifier(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
Parsing_status fully_resolve_identifier(std::shared_ptr<Identifier>& identifier);


// statement_parser.cpp
struct Statement;
std::shared_ptr<Statement> read_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
// FIXME: NYI:
Parsing_status fully_resolve_statement(std::shared_ptr<Statement> statement);


// using_parser.cpp
struct Using_statement;
std::shared_ptr<Using_statement> read_using_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
Parsing_status fully_resolve_using(std::shared_ptr<Using_statement> using_statement);
void resolve_imports(std::shared_ptr<Scope> scope);


// TODO:
std::shared_ptr<If_statement> read_if_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<For_statement> read_for_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<while_statement> read_while_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<return_statement> read_return_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<declaration_statement> read_declaration_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<assignment_statement> read_assignment_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope);





// TODO:
// literal_parser.cpp
// assignment_parser.cpp
// declaration_parser.cpp
// defer_parser.cpp
// for, if, while
// variable_expression_parser.cpp + helpers
// value_expression_parser.cpp + helpers
// (read_comma_separated_value_list?)






