#ifndef parser_h
#define parser_h

#include "abstx.h"
#include <string>

Static_scope parse_file(const std::string& file);
Static_scope parse_string(const std::string& string);


// for tests in main
#include "token.h"
Token const * read_paren(Token const * it);
Token const * read_bracket(Token const * it);
Token const * read_brace(Token const * it);


#endif