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

static bool is_eof(const Token_type& t) {
    return t == Token_type::EOF;
}

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

    bool is_eof() const { return ::is_eof(type); }
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
















/*
A Token_iterator holds pointers into a percieved list of Tokens and uses them to iterate through it.
This list has to be stored elsewhere, and cannot move, change or deallocate as long as the Token_iterator
  is alive, or as long as any pointers into it are still stored elsewhere. MANY POINTERS, HANDLE IT!

Normal usage is one of the following:
* Get the next token with eat_token(), then check if it's valid. If not, log an error.
* Get the next token with expect(...). This logs an error if the token wasn't what was expected.
    Check if the token is valid either manually, or through expect_failed()
* Get a future token with look_ahead(int) to check several tokens in a row. Then use eat_tokens() to eat them all at once.
*/

struct Token_iterator
{
    Token const * const start_token; // should always end with an EOF-token
    Token const * end_token = nullptr; // is updated when eof is found. Can be set manually to limit the search space.
    bool error = false;
    int index = 0;

    int known_min_size = 0; // optimization for lots of lookaheads

    Token_iterator(Token const * const start_token) : start_token{start_token} {
        ASSERT(start_token != nullptr);
    }

    Token_iterator(const std::vector<Token>& tokens) : start_token{&tokens[0]} {
        end_token = start_token + tokens.size()-1;
        ASSERT(end_token->is_eof());
    }

    // Returns the current token.
    Token current_token()
    {
        error = false;
        return start_token[index];
    }


    // Returns a token n steps ahead.
    // Expects the index to be inside the bounds of the token vector
    Token look_ahead(int n)
    {
        ASSERT(index+n >= 0);

        // update known_min_size if outdated
        if (end_token != nullptr) known_min_size = end_token - start_token;
        else if (known_min_size < index) known_min_size = index;

        // return values instant if possible
        if (index+n <= known_min_size) return start_token[index+n];
        if (index+n > known_min_size && end_token != nullptr) return *end_token;

        // go forward n steps and check each token for eof
        ASSERT(n > 0);
        int it = known_min_size;
        while (it < index+n && !start_token[it].is_eof())
            it++;

        known_min_size = it;
        if (start_token[it].is_eof())
            end_token = start_token+it;

        return start_token[it];
    }


    // Returns a token n steps back.
    Token look_back(int n) { return look_ahead(-n); }


    // Returns the current token and increments the index.
    Token eat_token()
    {
        Token t = current_token(); // sets error to false
        if (!t.is_eof())
            index++;
        return t;
    }


    // Increments the index by n.
    void eat_tokens(int n)
    {
        ASSERT(n > 0);

        if (known_min_size < index) known_min_size = index;
        error = false;

        if (index+n <= known_min_size) {
            index += n;

        } else if (end_token != nullptr) {
            known_min_size = end_token - start_token;
            index = known_min_size;

        } else {
            look_ahead(n); // updates known_min_size and end_token
            ASSERT(index+n <= known_min_size || end_token != nullptr);
            eat_tokens(n);
        }
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


