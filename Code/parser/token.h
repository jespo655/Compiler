#pragma once

#include "../utilities/assert.h"
#include "../utilities/error_handler.h"

#include <string>
#include <sstream>
#include <vector>

struct Token_context
{
    int line = 1;
    int position = 0;
    std::string file{};

    bool operator==(const Token_context& tc) const { return line==tc.line && position==tc.position && file==tc.file; }
    bool operator!=(const Token_context& tc) const { return !(*this==tc); }
    bool operator<(const Token_context& tc) const
    {
        ASSERT(file==tc.file, "TODO: decide what happens if comparing contexts from different files.");
        if (line < tc.line) return true;
        if (line == tc.line && position < tc.position) return true;
        return false;
    }
    bool operator<=(const Token_context& tc) const
    {
        ASSERT(file==tc.file, "TODO: decide what happens if comparing contexts from different files.");
        if (line < tc.line) return true;
        if (line == tc.line && position <= tc.position) return true;
        return false;
    }
    bool operator>(const Token_context& tc) const { return !(*this<=tc); }
    bool operator>=(const Token_context& tc) const { return !(*this<tc); }

    std::string toS() const {
        std::ostringstream oss;
        if (!file.empty()) {
            oss << "(In " << file << ", ";
        } else {
            oss << "(At ";
        }
        oss << "line " << line << ", position " << position << ")";
        return oss.str();
    }

};









#undef EOF // we want to use this for the Token_type struct

enum struct Token_type
{
    UNKNOWN,
    SYMBOL, // could be a multi character symbol (matches the longest one possible)
    IDENTIFIER,
    KEYWORD,
    COMPILER_COMMAND, // anything prefixed with '#'
    INTEGER,
    FLOAT, // number with decimal point '.', with a number string both before and after (1.0).
    STRING, // without the "" around
    BOOL, // true, false
    EOF // end of file
};

static std::string toS(const Token_type& t) {
    switch(t) {
        case Token_type::SYMBOL: return "SYMBOL";
        case Token_type::IDENTIFIER: return "IDENTIFIER";
        case Token_type::KEYWORD: return "KEYWORD";
        case Token_type::COMPILER_COMMAND: return "COMPILER_COMMAND";
        case Token_type::INTEGER: return "INTEGER";
        case Token_type::FLOAT: return "FLOAT";
        case Token_type::STRING: return "STRING";
        case Token_type::BOOL: return "BOOL";
        case Token_type::EOF: return "EOF";

        default: return "UNKNOWN";
    }
}

/*
All reserved words are called keywords
Keywords:

fn
if
elsif
else
then
for
while
struct
return
using
infix_operator
prefix_operator
suffix_operator
*/


struct Token
{
    Token_type type = Token_type::UNKNOWN;
    std::string token{};
    Token_context context{};

    Token() {}
    Token(const Token_type& type, const std::string token) : type{type}, token{token} {}
    Token(const std::string token, const Token_type& type) : type{type}, token{token} {}
    bool operator==(const Token& t) const { return type == t.type && token == t.token; }
    bool operator!=(const Token& t) const { return !(*this==t); }
    // bool operator<(const Token& t) const { return context < t.context; }
    // bool operator<=(const Token& t) const { return context <= t.context; }
    // bool operator>(const Token& t) const { return context > t.context; }
    // bool operator>=(const Token& t) const { return context >= t.context; }
};



/*
The Token_queue holds a list of Tokens and iterates through them.
Normal usage is one of the following:
* Get the next token with eat_token(), then check if it's valid. If not, log an error.
* Get the next token with expect(...). This logs an error if the token wasn't what was expected.
    Check if the token is valid either manually, or through expect_failed()
* Get a future token with look_ahead(int) to check several tokens in a row. Then use eat_tokens() to eat them all at once.
*/
struct Token_queue
{
    std::vector<Token> tokens; // should always end with an EOF-token
    int index = 0;
    bool error = false;


    // Returns the current token.
    Token current_token()
    {
        error = false;
        return tokens[index];
    }


    // Returns a token n steps ahead.
    // Expects the index to be inside the bounds of the token vector
    Token look_ahead(int n)
    {
        ASSERT(index+n >= 0);
        if (index+n >= tokens.size()) {
            error = true;
            return tokens[tokens.size()-1];
        }
        error = false;
        return tokens[index+n];
    }


    // Returns a token n steps back.
    Token look_back(int n) { return look_ahead(-n); }


    // Returns the current token and increments the index.
    Token eat_token()
    {
        Token t = current_token(); // sets error to false
        if (index < tokens.size()-1)
            index++;
        return t;
    }


    // Increments the index by n.
    void eat_tokens(int n)
    {
        ASSERT(n > 0);
        if (index+n >= tokens.size()) index = tokens.size();
        else index += n;
        error = false;
    }


    // Returns the current token and increments the index.
    // If the token type didn't match the expected type, also logs an error.
    Token expect(const Token_type& expected_type)
    {
        Token t = eat_token(); // sets error to false
        if (t.type != expected_type) {
            log_error("Expected token of type "+toS(expected_type)+", but found type "+toS(t.type), t.context);
            error = true;
        }
        return t;
    }


    // Returns the current token and increments the index.
    // If the token didn't match the expectation, also logs an error.
    Token expect(const Token_type& expected_type, std::string expected_token)
    {
        Token t = eat_token(); // sets error to false
        if (t.type != expected_type || t.token != expected_token) {
            log_error("Expected token \""+expected_token+"\" ("+toS(expected_type)+"), but found \""+t.token+"\" ("+toS(t.type)+")", t.context);
            error = true;
        }
        return t;
    }


    // Returns true if the last expect call failed.
    bool expect_failed() { return error; }
};

