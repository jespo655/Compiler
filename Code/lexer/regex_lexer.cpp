#include <regex>
#include "../parser/token.h"
#include <sstream> // ostringstream
#include <fstream> // reading from files

#include <string> // test suite
#include <vector> // test suite
#include "../utilities/assert.h" // test suite
#include "../utilities/error_handler.h"
#include <iostream> // debugging

#define RX_TEST

#ifdef EOF
#undef EOF
#endif
/*

An attempt on a more robust and clean lexer, easier to maintain and extend: Do all parsing with regex.
This way, rules can easily be added or removed, without chaning complicated code.

Drawbacks:
    * Probably worse efficiency for most code (might not be significant).
    * Harder to implement proper UTF-8 compliance (maybe, needs more research).


Approach 1:
    Read one line at the time from the file
    Use single-token regex, one token at the time
    Ensure that the match prefix is empty
    Pick out the token with match[0]
    Do some additional analysis to determine the type of token.

    Tokens that cannot be read will be collected in the match prefix.


Approach 2:
    Read one line at the time from the file
    Use several single-token regexes of different token types
    Use start of line anchor and check for whitespace, then capture group around the wanted token.
    Pick out the token with match[1]

    Tokens that cannot be read will not match any of these alternatives.


    NOTE:
    For HERE-string regex R"((\w+\b)(.*?)\b\1\b)" several lines need to be concatenated. Read one more line and concat until a match is found.

*/


// \w <=> [a-zA-z0-9_]


std::regex identifier_rx(R"(^\s*([a-zA-Z]\w*[\?\!]*))");
std::regex int_rx(R"(^\s*(\d+))"); // 1237
std::regex float_rx(R"(^\s*(\d+\.\d+))"); // 123.55324, might be expanded with exponent and other standard things
std::regex here_string_rx(R"(^\s*\b(\w+)\b(.*?)\b\1\b)"); // should use capture group 2
std::regex compiler_rx(R"(^\s*(\#[a-zA-Z]\w*[\?\!]*))"); // same as identifier but with leading '#'

/*
    // Symbols

    // infix
    |\*|\/|\+|\-|\%

    // comparison
    |==|<|>|<=|>=|!=

    // assignment
    // can be prefixed with an infix operator for direct assignment (like +=)
    |=

    // declaration
    |:

    // comments
    |\/\/
    |\/\*
    |\*\/

    // cast operator
    |_

    // scope stuff
    |\(|\)|\[|\]|\{|\}

    // other
    |;|,|\.|\.\.|->|$

    // reserved for future use
    |?|!|&|#
*/
std::regex symbol_rx(R"(^\s*(\*|\/|\+|\%|==|<=|>=|<|>|!=|=|:|\_|\(|\)|\[|\]|\{|\}|\;|\,|\.\.\.|\.\.|\.|\->|\-|\$|\?|\!|\&|\#))"); // Pseudo-sorted, long symbols should be first in the rx (e.g. '..' must be before '.')

/*
    // Keywords

    |for|in|by
    |if|elsif|else|then
    |while

    |fn|return
    |cast
    |struct

    |defer
    |operator
*/
std::regex keyword_rx(R"(^\s*(for|in|by|if|elsif|else|then|while|fn|return|cast|struct|defer|inline|operator)\b)");

std::regex bool_rx(R"("^\s*(true|false)\b)");
std::regex string_rx(R"(^\s*\"(([^\\]*?(\\.)*?)*?)\")");

std::regex whitespace_rx(R"(^\s*$)");

std::regex comment_rx(R"(^\s*(\/\/|\/\*|\*\/))");






// returns true if a line could be read.
// false = error.
bool next_line(std::string& buffer, std::istream& input)
{
    if (input.eof()) return false;
    std::getline(input, buffer);
    return true;
}





// retrns number of characters consumed
bool try_match(std::string& str, Token& token, std::vector<Token>& tokens, const std::regex& rx, const Token_type& type, int capture_group = 1) {
    std::smatch match;
    if (regex_search(str, match, rx)) {
        token.token = match[capture_group];
        token.type = type;
        str = match.suffix();
        tokens.push_back(token);
        token.context.position += match.length();
        return true;
    }
    return false;
}


std::vector<Token> read_tokens(std::istream& input, const std::string& file_name)
{
    Token t;
    t.context.line = 0;
    t.context.position = 0;
    t.context.file = file_name;

    std::string current_line = "";
    std::vector<Token> tokens;

    // check the different regexes
    std::smatch match;
    bool matched;

    while(1)  {

        if (regex_match(current_line, match, whitespace_rx)) {
            t.context.line++;
            t.context.position=1;
            if (!next_line(current_line, input)) return tokens;
            continue;
        }

        if (try_match(current_line, t, tokens, compiler_rx, Token_type::COMPILER_COMMAND)) { // before symbol
            if (t.token == "#string") {
                // if a match cannot be found on the current line, read a new line, concat (including newline), try again
                if(!try_match(current_line, t, tokens, here_string_rx, Token_type::STRING)) {

                    // No match on the current line: check line for line, concatenating the result string as we go
                    // Stop at the first delimiter token.

                    std::smatch match;
                    bool delim_match = regex_search(current_line, match, identifier_rx);
                    ASSERT(delim_match);
                    std::string delimiter = match[1];
                    std::regex delimiter_rx("^(.*?)\\b"+delimiter+"\\b"); // build delimiter regex
                    std::ostringstream sb(match.suffix()); // start building the resulting here string

                    int added_lines = 0;
                    while(1) {
                        added_lines++;
                        if (!next_line(current_line, input)) {
                            t.context.line += added_lines;
                            t.context.position = 1;
                            log_error("Unexpected end of file, expected here-string delimiter '"+delimiter+"'", t.context);
                            return tokens;
                        }

                        if(regex_search(current_line, match, delimiter_rx)) {
                            sb << std::endl << match[1];
                            t.token = sb.str();
                            tokens.push_back(t);
                            t.context.line += added_lines;
                            t.context.position = 1 + match.length();
                            break;
                        }

                        sb << std::endl << current_line;
                    }
                }
            }
            matched = true;
        }

        matched |= try_match(current_line, t, tokens, string_rx, Token_type::STRING);

        if(try_match(current_line, t, tokens, symbol_rx, Token_type::SYMBOL)) {
            if (t.token == "//") {
                current_line = "";
                continue;
            }
            if (t.token == "*/") {
                log_error("Unmatched '*/'", t.context);
            }
            if (t.token == "/*") {
                int comment_depth = 1;
                bool comment_error = false;
                while (comment_depth > 0) {
                    if (regex_search(current_line, match, comment_rx)) {
                        t.context.position += match.length();
                        if (t.token == "//") {
                            t.context.line++;
                            t.context.position=1;
                            if (!next_line(current_line, input)) comment_error = true;
                        }
                        else if (t.token == "/*") comment_depth++;
                        else if (t.token == "*/") comment_depth--;
                    } else {
                        t.context.line++;
                        t.context.position=1;
                        if (!next_line(current_line, input)) comment_error = true;
                    }
                    if (comment_error) {
                        log_error("Unmatched '/*' at end of file", t.context);
                        return tokens;
                    }
                }
            }
            matched = true;
        }

        matched |= try_match(current_line, t, tokens, bool_rx, Token_type::BOOL); // before identifier
        matched |= try_match(current_line, t, tokens, keyword_rx, Token_type::KEYWORD); // before identifier

        matched |= try_match(current_line, t, tokens, int_rx, Token_type::INTEGER);
        matched |= try_match(current_line, t, tokens, float_rx, Token_type::FLOAT);
        matched |= try_match(current_line, t, tokens, identifier_rx, Token_type::IDENTIFIER);

        if (!matched) {
            // TODO: @log_error
        }

    }
}












void add_eof_token(std::vector<Token>& tokens)
{
    Token t;
    t.token = "eof";
    t.type = Token_type::EOF; // this is what everything should break on in the parser
    t.context.position = 1;
    if (!tokens.empty())
        t.context.line = tokens.back().context.line + 1;
    tokens.push_back(t);
}


std::vector<Token> get_tokens_from_string(const std::string& source, const std::string& string_name)
{
    std::istringstream iss{source};
    std::vector<Token> tokens = read_tokens(iss, string_name);
    add_eof_token(tokens);
    return tokens;
}

std::vector<Token> get_tokens_from_file(const std::string& source_file)
{
    std::ifstream file;
    file.open(source_file);
    std::vector<Token> tokens;
    if (file.is_open()) {
        tokens = read_tokens(file, source_file);
    } else {
        std::cout << "Unable to open file \"" << source_file << "\"" << std::endl;
    }
    file.close();
    add_eof_token(tokens);
    return tokens;
}












#ifdef RX_TEST

// Test suite
void rx_test(const std::regex& rx, std::string text, const std::vector<std::string>& expected_matches, const std::string test_name = "", const int capture_group = 1)
{
    std::cout << "Running test" << (test_name==""?"...":" "+test_name) << std::endl;

    std::regex non_whitespace_rx(R"([^\s]+)");
    for (const std::string& expected : expected_matches)
    {
        std::smatch match;
        std::smatch non_whitespace_match;
        regex_search(text, match, rx);

        std::string prefix = match.prefix();
        if(regex_search(prefix, non_whitespace_match, non_whitespace_rx)) {
            std::cout << "Rx test failed. Found unexpected token \"" << non_whitespace_match[0] << "\"." << std::endl;
            ASSERT(false);
        }
        if(match[capture_group] != expected) {
            std::cout << "Rx test failed. Expected \"" << expected << "\", but found \"" << match[0] << "\"." << std::endl;
            ASSERT(false);
        }
        text = match.suffix();
    }
}


void print_first_match(const std::regex& rx, std::string text)
{
    std::smatch match;
    if(regex_search(text, match, rx)) {
        for (int i = 0; i < match.size(); ++i) {
            std::cout << i << ": " << match[i] << std::endl;
        }
    } else {
        std::cout << "No match." << std::endl;
    }
}


void rx_test_suite()
{
    std::cout << " -- Test suite: regex matches -- " << std::endl;

    rx_test(identifier_rx,
        "lots of identifiers some_with_underscore or_numb3rs or_ending_with_questionmark?!??",
        {"lots", "of", "identifiers", "some_with_underscore", "or_numb3rs", "or_ending_with_questionmark?!??"},
        "identifier test");

    rx_test(int_rx,
        "872 098 020 313 92382938 123182739387888",
        {"872", "098", "020", "313", "92382938", "123182739387888"},
        "int test");

    rx_test(float_rx,
        "132.567 123.78 7654.345 321.567 2356654.2456 24567898765446886543.234567897654345",
        {"132.567", "123.78", "7654.345", "321.567", "2356654.2456", "24567898765446886543.234567897654345"},
        "float test");

    rx_test(here_string_rx,
        "HERE askdjh ki28 random text that shouldnt matter, including the word ALMOSTHERE, but not yet. OK here's the end: HERE plus some more text HERE",
        {" askdjh ki28 random text that shouldnt matter, including the word ALMOSTHERE, but not yet. OK here's the end: "},
        "here string test", 2);

    rx_test(symbol_rx,
        "*/+-%==<><=>=!==:_()[]{};,...->$?!&#",
        {"*", "/", "+", "-", "%", "==", "<", ">", "<=", ">=", "!=", "=", ":", "_", "(", ")", "[", "]", "{", "}", ";", ",", "...", "->", "$", "?", "!", "&", "#"},
        "symbol test");

    rx_test(compiler_rx,
        "#if #include #string",
        {"#if", "#include", "#string"},
        "compiler test");

    // print_first_match(string_rx, "\"this is an escaped string \\\" string\" with some extra \"");

    rx_test(string_rx,
        "\"this is a string\" with some extra \"",
        {"this is a string"},
        "string test");

    rx_test(string_rx,
        "\"this is an escaped string \\\" string\" with some extra \"",
        {"this is an escaped string \\\" string"},
        "escaped string test");

    std::cout << "All tests passed." << std::endl;
}

void print_tokens(const std::vector<Token>& tokens) {
    if (tokens.empty()) {
        std::cout << "-- List of tokens is empty. --\n";
        return;
    }
    std::cout << "-- List of tokens: --\n";
    std::cout << tokens[0].toS();
    for (int i = 1; i < tokens.size(); ++i) {
        std::cout << ", " << tokens[0].toS();
    }
    std::cout << std::endl;
}



int main(int argc, char** argv) {
    // rx_test_suite();

    if (argc != 2) {
        printf("Invalid amount of arguments (%d). Supply a single filename.\n", argc-1);
        return EXIT_FAILURE;
    }

    std::vector<Token> tokens = get_tokens_from_file(argv[1]);
    print_tokens(tokens);
}

#endif


