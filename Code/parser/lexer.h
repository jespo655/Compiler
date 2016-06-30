#pragma once

#include <string>
#include <vector>

struct Token;


/*
The lexer takes a string and tokenize it.
The tokens are stored in a vector, which then whould be used with Token_queue
*/

std::vector<Token> get_tokens_from_file(const std::string& source_file);
std::vector<Token> get_tokens_from_string(const std::string& source, const std::string& string_name = "");