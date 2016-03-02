#ifndef lexer_h
#define lexer_h

#include <string>
#include <iostream> // istream
#include <vector>



enum struct Token_type
{
    UNKNOWN,
    SYMBOL,
    IDENTIFIER, // can also be a keyword
    KEYWORD,
    INTEGER,
    FLOAT,
    STRING // without the "" around
};

struct Token_context
{
    int line = 1;
    int position = 0;
    std::string file{};
};


struct Token
{
    std::string token{};
    Token_context context{};
    Token_type type = Token_type::UNKNOWN;
};



class Stream
{
public:
    char peek(); // peeks the first character
    char pop(); // pops the first character
    std::string peek_utf8();
    std::string pop_utf8();
    bool empty();
    std::string read_line();

    Stream() : stream{std::cin} {}
    Stream(std::istream& s, std::string source = "UNKNOWN") : stream{s} { context.file = source; read_next_utf8_token(); }

    bool operator==(const Stream& o) const { return &stream == &o.stream; };

    Token_context context{};
private:
    std::istream& stream;

    void read_next_utf8_token();
    std::string last_utf8_token = " ";
    bool has_tokens = true;
};




// will keep the last token when empty
class Token_iterator
{
public:
    Token& peek() { return current_token; }
    Token pop()
    {
        Token t = current_token;
        if (!empty)
            current_token = read_next_token();
        return t;
    }
    Token pop(const std::string& expectation_str); // logs unexpected end of file error
    bool has_tokens() { return !empty; }

    // operators for use in range for loops:
    Token& operator*() { return current_token; }
    Token_iterator operator++() // prefix ++
    {
        if (!empty)
            current_token = read_next_token();
        return *this;
    }
    bool operator==(const Token_iterator& o) const {
        if (empty != o.empty) return false;
        if (empty && o.empty) return true;
        return stream == o.stream;
    }
    bool operator!=(const Token_iterator& o) const { return !(*this==o); }

    Token_iterator() : empty{true} {}
    Token_iterator(Stream& s) : stream{s} { current_token = read_next_token(); }

private:

    Token read_next_token();
    bool empty = false;
    Token current_token{};
    Stream stream;

};

Token_iterator begin(Stream& s);
Token_iterator end(Stream& s);

std::vector<Token> get_tokens(const std::string& source_file);

#endif