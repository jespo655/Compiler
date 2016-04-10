#ifndef parser_h
#define parser_h

#include <vector>
#include "lexer.h"
#include <memory> // std::unique_ptr
#include "abstx.h"


// for tests in main
Token const * read_paren(Token const * it);
Token const * read_bracket(Token const * it);
Token const * read_brace(Token const * it);



struct Token_range
{
    Token const* start_token;
    Token const* end_token;
    virtual ~Token_range() {}
};



struct Dependency
{
    Dynamic_statement* statement;
    Token const * unknown_identifier_token; // the type of this identifier has to be know in order to resolve the statement
};



Static_scope parse_file(const std::string& file);
Static_scope parse_string(const std::string& string);


#endif