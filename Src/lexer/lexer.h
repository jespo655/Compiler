#pragma once

#include <string>

#include "../utilities/sequence.h"

struct Token;
struct Token_context;

/*
The lexer takes a string and tokenizes it.
The tokens are stored in a vector, which then can be iterated through and parsed.
The list of tokens always ends with an eof-token.
*/
Seq<Token> get_tokens_from_file(const std::string& source_file);
Seq<Token> get_tokens_from_string(const std::string& source, const std::string& string_name = "");
Seq<Token> get_tokens_from_string(const std::string& source, const Token_context& string_context);
