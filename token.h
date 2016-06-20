#ifndef token_h
#define token_h

#include <string>
#include "assert.h"


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
};









#undef EOF // we want to use this for the Token_type struct

enum struct Token_type
{
    UNKNOWN,
    SYMBOL,
    IDENTIFIER,
    KEYWORD,
    INTEGER,
    FLOAT,
    STRING, // without the "" around
    BOOL,
    EOF // end of file
};

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


std::ostream& operator << (std::ostream& os, const Token_context& context);
std::ostream& operator << (std::ostream& os, const Token_type& context);



#endif
