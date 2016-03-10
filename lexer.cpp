#include "lexer.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include "error_handler.h"
using namespace std;

/*
    read file (or other source of plain-text code)
    tokenize it and make a token iterator
*/

void add_eof_token(vector<Token>& tokens)
{
    Token t;
    t.token = "eof";
    t.type = Token_type::UNKNOWN; // this is what everything should break on in the parser
    t.context.position = 1;
    if (!tokens.empty())
        t.context.line = tokens.back().context.line + 1;
    tokens.push_back(t);
}


std::vector<Token> get_tokens_from_string(const std::string& source, const std::string& string_name)
{
    istringstream iss{source};
    Stream s{iss,string_name};
    vector<Token> tokens;
    for (Token& t : s) {
        tokens.push_back(t);
    }

    add_eof_token(tokens);
    return tokens;
}



std::vector<Token> get_tokens_from_file(const std::string& source_file)
{
    ifstream file;
    file.open(source_file);
    Stream s{file,source_file};

    vector<Token> tokens;
    for (Token& t : s) {
        tokens.push_back(t);
    }

    file.close();

    add_eof_token(tokens);
    return tokens;
}













Token_iterator begin(Stream& s)
{
    return Token_iterator(s);
}

Token_iterator end(Stream& s)
{
    return Token_iterator();
}


Token Token_iterator::pop(const std::string& expectation_str)
{
    if (empty) {
        log_error("Unexpected end of file: "+expectation_str,peek().context);
        return peek();
    }
    return pop();
}






bool is_line_ending(char c)
{
    return c == '\n' || c == '\r';
}



char Stream::peek()
{
    return last_utf8_token[0];
}

char Stream::pop()
{
    char c = last_utf8_token[0];
    read_next_utf8_token();
    return c;
}

string Stream::read_line()
{
    string s{};
    if (!is_line_ending(last_utf8_token[0])) {
        context.line++;
        context.position = 0;
        getline(stream, s);
    }
    read_next_utf8_token();
    return s;
}

bool Stream::empty()
{
    return !has_tokens;
}


string Stream::peek_utf8()
{
    return last_utf8_token;
}

string Stream::pop_utf8()
{
    string s = last_utf8_token;
    read_next_utf8_token();
    return s;
}


int get_number_of_bytes(signed char first_utf8_byte)
{
    if (first_utf8_byte <= (char)0xdf) return 2;
    if (first_utf8_byte <= (char)0xef) return 3;
    if (first_utf8_byte <= (char)0xf7) return 4;
    if (first_utf8_byte <= (char)0xfb) return 5;
    if (first_utf8_byte <= (char)0xfd) return 6;
    if (first_utf8_byte <= (char)0xff) return 0; // error
    if (first_utf8_byte <= (char)0x7f) return 1;
}

bool starts_with_0x10(signed char c) // todo: shouldn't be named "0x10"
{
    return c >= (char)0x80 && c <= (char)0xbf;
}

#include <iomanip>

void Stream::read_next_utf8_token()
{
    char c;
    if (!stream.get(c)) {
        has_tokens = false;
        last_utf8_token = " ";
        return;
    }

    if (is_line_ending(c)) {
        context.line++;
        context.position = 0;
    } else {
        context.position++;
    }

    // assert that the byte does not start with with 0x10
    if (starts_with_0x10(c)) {
        ostringstream oss;
        oss << "Found malformed byte in beginning of utf-8 character sequence: " << setw(4) << hex << (int)c << ", char: " << c;
        log_error(oss.str(),context);
    }

    string s{c};
    int additional_bytes = get_number_of_bytes(c)-1;
    for (int i = 0; i < additional_bytes; ++i)
    {
        if (!stream.get(c)) {
            ostringstream oss;
            oss << "Unexpected end of file in the middle of utf-8 character sequence. Expected "
                << (additional_bytes+1) << + " bytes but only found " << (i+1);
            log_error(oss.str(),context);
            has_tokens = false;
            last_utf8_token = " ";
            return;
        }
        // assert that the byte starts with 0x10
        if (!starts_with_0x10(c)) {
            ostringstream oss;
            oss << "Found malformed byte in the middle of utf-8 character sequence: " << hex << (int)c << ", char: " << c;
            oss << " (started with character " << s[0] << ", hex " << setw(4) << hex << (int)s[0] << ")";
            log_error(oss.str(),context);
            stream.putback(c);
            break;
        }
        s += c;
    }
    last_utf8_token = move(s);
}










void skip_line_comment(Stream& s)
{
    s.read_line();
}

void skip_block_comment(Stream& s)
{
    // read until we find the corresponding "*/"
    int nested_comments = 1;
    while (!s.empty() && nested_comments > 0) {
        char c = s.pop();
        if (c == '*' && s.pop() == '/') // "*/"
            nested_comments--;
        else if (c == '/' && s.pop() == '*') // "/*"
            nested_comments++;
    }
    if (nested_comments > 0 && s.empty())
        log_error("Unexpected end of file in the middle of comment ", s.context);
}





// TODO: logical operators

vector<string> symbols
{
    // infix: takes left and right side, performs a operator on it and returns the result
    // a [infix] b is equivalent with [infix](a,b)->T
    "*", // can also be a pointer dereference
    "/",
    "+",
    "-",
    "%", // should be "%", ignore the pink shit

    // comparison
    "==",
    "<",
    ">",
    "<=",
    ">=",
    "!=",

    // assignment (always between LHS and RHS)
    "=",
    "*=",
    "/=",
    "+=",
    "-=",
    "%=", // should be "%=", ignore the pink shit
    ":", // define - the identifier cannot be assigned something else later. (struct members can still be changed)

    // comments
    "//",
    "/*",
    "*/",

    // cast operator
    "_",

    // scope stuff
    "(", ")",
    "[", "]",
    "{", "}",

    // other
    ";", // end of statement
    ",", // separator in lists
    ".", // accessing data from a struct
    "..", // range operator
    "->", // starts the return list in function signatures

    // reserved for future use
    "?", // maybe for pointers?
    "!", // maybe for pointers?
    "&",
    "#",
};

vector<string> keywords
{
    "for",
    "in",
    "by",

    "if",
    "else",

    "while",

    "fn",
    "return",

    "cast", // syntactic sugar for constructor overloading
    // "implicit_cast",
    // "const",
    "struct",

    "defer",
    "infix_operator", // operator overloading
};


bool is_whitespace(char c)
{
    return is_line_ending(c) || c == ' ' || c == '\t';
}

bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

bool is_assignment_operator(const string& s)
{
    return s == "=" || s == "*=" || s == "/=" || s == "+=" || s == "-=" || s == "%%=";
}

bool is_valid_identifier_name_start(char c)
{
    // todo: accept any utf-8 character?
    // maybe not as the first character
    return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

bool is_valid_char_in_identifier_name(char c)
{
    // todo: accept any utf-8 character?
    // except whitespace and symbols, though.
    return is_valid_identifier_name_start(c) || c == '_' || is_digit(c);
}

bool is_keyword(const string& s)
{
    for (string& k : keywords)
        if (s == k) return true;
    return false;
}

// sort(symbols.begin(),symbols.end())


void read_symbol(Stream& s, Token& t)
{
    vector<string> possible_symbols = symbols;
    vector<string> next_possible_symbols{};
    int pos = 0;
    t.token = "";
    t.type = Token_type::SYMBOL;
    while (!s.empty() && possible_symbols.size() > 1 || possible_symbols.size() == 1 && possible_symbols[0] != t.token)
    {
        string str = s.peek_utf8();
        for (int i = 0; i < str.size(); ++i) {
            char c = str[i];
            bool should_pop = false;
            for (string& s : possible_symbols) {
                if (s.size() == pos) {
                    next_possible_symbols.push_back(s);
                }
                if (s.size() > pos && s[pos] == c) {
                    next_possible_symbols.push_back(s);
                    should_pop = true;
                }
            }
            if (should_pop)
                t.token += s.pop();
            possible_symbols = next_possible_symbols;
            next_possible_symbols.clear();
            pos++;
        }
    }

    if (possible_symbols.size() == 1) {
        // OK!
        return;
    }

    if (!t.token.empty()) {
        ostringstream oss;
        oss << "Lexer error: Could not parse character sequence: \"" << t.token << '\"';
        log_error(oss.str(), s.context);
        t.type = Token_type::UNKNOWN;
        return;
    }
}



string read_identifier_name(Stream& s)
{
    if (s.empty()) return "";
    string str = s.peek_utf8();
    if (!is_valid_identifier_name_start(str[0])) return "";
    string token{};
    token += s.pop_utf8();
    while (!s.empty() && is_valid_char_in_identifier_name(s.peek_utf8()[0])) {
        token += s.pop_utf8();
    }
    return token;
}


string read_string(Stream& s)
{
    if (s.peek_utf8() != "\"") return "";
    string str = s.pop_utf8();
    string token{};
    bool escaped = false;
    while(!s.empty()) {
        escaped = (str == "\\");
        str = s.pop_utf8();
        if (str == "\"" && !escaped)
            return token;
        token += str;
    }
    log_error("Lexer error: Unexpected end of file in the middle of a string", s.context);
    return "";
}

string read_number(Stream& s)
{
    string token{};
    while (!s.empty() && is_digit(s.peek_utf8()[0]))
        token += s.pop_utf8();
    return token;
}

void read_number_token(Stream& s, Token& t)
{
    if (s.empty() || !is_digit(s.peek_utf8()[0]))
        return;
    t.token = "";
    t.type = Token_type::INTEGER;
    t.token += read_number(s);
    if (s.peek_utf8() == ".")
    {
        t.type = Token_type::FLOAT;
        // check for decimals
        t.token += s.pop_utf8();
        if (!is_digit(s.peek())) {
            log_error("Lexer error: no decimals after decimal point!", s.context);
            t.type = Token_type::UNKNOWN;
            return;
        }
        t.token += read_number(s);
    }
}

/*
TODO: Do the entire context evaluation on the next step
*/

Token Token_iterator::read_next_token()
{

    // ignore whitespace
    while (!stream.empty() && is_whitespace(stream.peek_utf8()[0]))
        stream.pop_utf8();

    Token t;
    t.context = stream.context;

    // end of file?
    if (stream.empty())
    {
        empty = true;
        t.token = "";
        t.type = Token_type::UNKNOWN;
        return t;
    }

    if (stream.peek_utf8() == "\"")
    {
        // read string
        t.token = read_string(stream);
        t.type = Token_type::STRING;
        return t;
    }

    // try to read symbol
    read_symbol(stream,t);
    if (!t.token.empty())
    {
        if (t.token == "//") {
            skip_line_comment(stream);
            return read_next_token();
        }
        if (t.token == "/*") {
            skip_block_comment(stream);
            return read_next_token();
        }
        return t;
    }

    // try to read identifier
    t.token = read_identifier_name(stream);
    if (!t.token.empty())
    {
        t.type = Token_type::IDENTIFIER;
        if (is_keyword(t.token))
            t.type = Token_type::KEYWORD;
        return t;
    }

    // try to read number
    read_number_token(stream, t);
    if (!t.token.empty())
    {
        return t;
    }

    // strange stuff is happening, can't parse code file
    ostringstream oss;
    oss << "Lexer error: Could not parse character sequence: \"" << stream.peek_utf8() << '\"';
    log_error(oss.str(), stream.context);
    t.type = Token_type::UNKNOWN;
    t.context = stream.context;
    t.token = stream.pop_utf8();
    return t;
}
