#include "lexer.h"
#include "../parser/token.h"
#include "../utilities/error_handler.h"
#include "../utilities/assert.h"
#include <regex>
#include <sstream>
#include <fstream>
#include <string>
#include <iostream> // debugging, error message for unable to open file
#include <algorithm>

#ifdef EOF
#undef EOF
#endif

/*

An attempt on a more robust and clean lexer, easier to maintain and extend: Do all parsing with regex.
This way, rules can easily be added or removed, without chaning complicated code.

Drawbacks:
    * Probably worse efficiency for most code (might not be significant).
    * Harder to implement proper UTF-8 compliance (maybe, needs more research).

Approach:
    Read one line at the time from the file
    Use several single-token regexes of different token types
    Use start of line anchor and check for whitespace, then capture group around the wanted token.
    Pick out the token with match[1]

    Tokens that cannot be read will not match any of these alternatives.

    NOTE:
    For HERE-string regex R"((\w+\b)(.*?)\b\1\b)" several lines need to be concatenated. Read one more line and concat until a match is found.

Naming:
    A scanner reads files but doesn't process the text.
    A tokenizer splits the text into tokens.
    A lexer splits the text into tokens and adds additional information, such as token type and context.
Conclusion: This is a lexer.

TODO: Error messages for incomplete strings

*/

// \w <=> [a-zA-z0-9_]

std::regex only_whitespace_rx(R"(^\s*$)");
std::regex whitespace_rx(R"(^\s+)");

std::regex identifier_rx(R"(^([a-zA-Z]\w*[\?\!]*)\s*)");
std::regex int_rx(R"(^(\d+)\s*)"); // 1237
std::regex float_rx(R"(^(\d+\.\d+)\s*)"); // 123.55324, might be expanded with exponent and other standard things
std::regex compiler_rx(R"(^(\#[a-zA-Z]\w*[\?\!]*)\s*)"); // same as identifier but with leading '#'

std::regex comment_start_rx(R"(^\s*(\/\/|\/\*|\*\/)\s*)");
std::regex comment_end_rx(R"(^.*?(\/\/|\/\*|\*\/)\s*)");

std::regex symbol_rx(R"(^(\*|\/|\+|\%|==|<=|>=|<|>|!=|=|:|\_|\(|\)|\[|\]|\{|\}|\;|\,|\.\.\.|\.\.|\.|\->|\-|\$|\?|\!|\&|\#|\')\s*)"); // Pseudo-sorted, long symbols should be first in the rx (e.g. '..' must be before '.')

// booleans and keywords are subsets of identifiers.
std::regex bool_rx(R"(^(true|false)$)");
std::regex keyword_rx(R"(^(for|in|by|if|elsif|else|then|while|fn|return|cast|struct|defer|inline|operator)$)");
// Additional possible keywords: implicit_cast, const

std::regex string_start_rx(R"(^\")");
std::regex string_rx(R"(^\"([^\\\"]*(\\.[^\\\"]*)*)\"\s*)"); // currently has to be on a single line only
std::regex here_string_rx(R"(^\b([a-zA-Z]\w*[\?\!]*)\b(.*?)\b\1\b\s*)"); // Delimiter is a regular identifier. Should use capture group 2 to capture the string


// returns true if a line could be read.
// false = error.
bool next_line(std::string& buffer, std::istream& input, Token_context& context)
{
    if (input.eof()) return false;
    std::getline(input, buffer);
    context.line++;
    context.position=1;
    std::smatch match;
    if (regex_search(buffer, match, whitespace_rx)) {
        context.position += match.length();
        buffer = match.suffix();
    }
    return true;
}

// returns true if matched
bool try_match(std::string& str, Token& t, Seq<Token>& tokens, const std::regex& rx, const Token_type& type, int capture_group = 1) {
    std::smatch match;
    if (regex_search(str, match, rx)) {
        t.token = match[capture_group];
        t.type = type;
        if (t.type == Token_type::COMPILER_COMMAND) std::transform(t.token.begin(), t.token.end(), t.token.begin(), ::tolower);
        str = match.suffix();
        tokens.add(t);
        t.context.position += match.length();
        return true;
    }
    return false;
}


Seq<Token> read_tokens(std::istream& input, Token_context initial_context)
{
    Token t;
    t.context = initial_context;

    std::string current_line = "";
    Seq<Token> tokens;

    std::smatch match;
    bool matched;

    while(1)  {

        matched = false;

        if (regex_match(current_line, match, only_whitespace_rx)) {
            if (!next_line(current_line, input, t.context)) return tokens;
            if (initial_context.line != 0 || initial_context.position != 0) {
                t.context = initial_context;
                initial_context.line = 0;
                initial_context.position = 0;
            }
            continue;
        }

        if (regex_search(current_line, match, comment_start_rx)) { // before symbol
            if (match[1] == "//") {
                current_line = "";
                continue;
            }
            if (match[1] == "*/") {
                log_error("Unmatched '*/'", t.context);
                current_line = match.suffix();
            }
            if (match[1] == "/*") { // nested block comments
                Token_context comment_start = t.context; // remember in case of error
                int comment_depth = 1;
                bool comment_error = false;
                current_line = match.suffix();
                while (comment_depth > 0) {
                    if (regex_search(current_line, match, comment_end_rx)) {
                        t.context.position += match.length();
                        if (match[1] == "//") {
                            if (!next_line(current_line, input, t.context)) comment_error = true;
                        }
                        else if (match[1] == "/*") {
                            comment_depth++;
                            current_line = match.suffix();
                        }
                        else if (match[1] == "*/") {
                            comment_depth--;
                            current_line = match.suffix();
                        }
                    } else {
                        if (!next_line(current_line, input, t.context)) comment_error = true;
                    }
                    if (comment_error) {
                        log_error("Unmatched '/*' at end of file", t.context);
                        add_note("Comment started here", comment_start);
                        return tokens;
                    }
                }
            }
            continue;
        }

        if (try_match(current_line, t, tokens, compiler_rx, Token_type::COMPILER_COMMAND)) { // before symbol

            if (t.token == "#string") { // Here-string
                tokens.remove_last(); // delete the #string token - just insert the string literal

                // if a match cannot be found on the current line, read a new line, concat (including newline), try again
                if(!try_match(current_line, t, tokens, here_string_rx, Token_type::STRING, 2)) {

                    // No match on the current line: check line for line, concatenating the result string as we go
                    // Stop at the first delimiter token.

                    Token_context new_context = t.context; // save the old context for later

                    std::smatch match;
                    bool delim_match = false;
                    while (!delim_match) {
                        delim_match = regex_search(current_line, match, identifier_rx);
                        if (!delim_match) {
                            if (regex_match(current_line, match, only_whitespace_rx)) {
                                // check next line
                                if (!next_line(current_line, input, new_context)) {
                                    log_error("Unexpected end of file, expected here-string delimiter identifier", new_context);
                                    return tokens;
                                }
                            } else {
                                log_error("Unexpected token, expected here-string delimiter identifier", new_context);
                                return tokens;
                            }
                        }
                    }
                    ASSERT(delim_match);
                    std::string delimiter = match[1];
                    std::regex delimiter_rx("^(.*?)\\b"+delimiter+"\\b"); // build delimiter regex
                    std::ostringstream sb(match.suffix()); // start building the resulting here string

                    while(1) {
                        if (!next_line(current_line, input, new_context)) {
                            log_error("Unexpected end of file, expected here-string delimiter '"+delimiter+"'", new_context);
                            return tokens;
                        }

                        if(regex_search(current_line, match, delimiter_rx)) {
                            sb << std::endl << match[1];
                            t.token = sb.str();
                            t.type = Token_type::STRING;
                            tokens.add(t); // this still has the old context
                            new_context.position = 1 + match.length();
                            t.context = new_context; // update to the new context
                            current_line = match.suffix(); // update current line
                            break;
                        }

                        sb << std::endl << current_line;
                    }
                }
            }
            continue;
        }

        if (regex_search(current_line, string_start_rx)) {
            if (!try_match(current_line, t, tokens, string_rx, Token_type::STRING)) {
                log_error("Missing '\"' at end of string", t.context);
                add_note("Strings must be terminated before end of line.");
                current_line = "";
            }
            continue;
        }

        if(try_match(current_line, t, tokens, symbol_rx, Token_type::SYMBOL)) continue;
        if(try_match(current_line, t, tokens, float_rx, Token_type::FLOAT)) continue;
        if(try_match(current_line, t, tokens, int_rx, Token_type::INTEGER)) continue;

        if (regex_search(current_line, match, identifier_rx)) {
            t.token = match[1];
            if (regex_match(t.token, keyword_rx)) {
                t.type = Token_type::KEYWORD;
            } else if (regex_match(t.token, bool_rx)) {
                t.type = Token_type::BOOL;
            } else {
                t.type = Token_type::IDENTIFIER;
            }
            tokens.add(t);
            t.context.position += match.length();
            current_line = match.suffix();
            continue;
        }

        // We didn't match anything -> error.
        log_error("Could not match token; ignoring until end of line.",t.context);
        add_note("Ignored characters: "+current_line);
        current_line = "";
    }
}

void add_eof_token(Seq<Token>& tokens, const std::string& file_name)
{
    Token t;
    t.token = "eof";
    t.type = Token_type::EOF; // this is what everything should break on in the parser
    t.context.file = file_name;
    t.context.position = 1;
    t.context.line = !tokens.empty() ? tokens.get(tokens.size-1).context.line + 1 : 1;
    tokens.add(t);
}

Seq<Token> get_tokens_from_string(const std::string& source, const Token_context& initial_context)
{
    std::istringstream iss{source};
    Seq<Token> tokens = read_tokens(iss, initial_context);
    add_eof_token(tokens, initial_context.file);
    return tokens;
}

Seq<Token> get_tokens_from_string(const std::string& source, const std::string& string_name)
{
    Token_context initial_context;
    initial_context.file = string_name;
    return get_tokens_from_string(source, initial_context);
}

Seq<Token> get_tokens_from_file(const std::string& source_file)
{
    std::ifstream file;
    file.open(source_file);
    Seq<Token> tokens;
    if (file.is_open()) {
        Token_context initial_context;
        initial_context.file = source_file;
        tokens = read_tokens(file, initial_context);
    } else {
        std::cout << "Unable to open file \"" << source_file << "\"" << std::endl; // @todo: this should be a compile error
    }
    file.close();
    add_eof_token(tokens, source_file);
    return tokens;
}













#ifdef TEST

// Test suite
void rx_test(const std::regex& rx, std::string text, const Seq<std::string>& expected_matches, const std::string test_name = "", const int capture_group = 1)
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

void print_tokens(const Seq<Token>& tokens) {
    if (tokens.empty()) {
        std::cout << "-- List of tokens is empty. --\n";
        return;
    }
    std::cout << "-- List of tokens: --\n";
    for (int i = 0; i < tokens.size(); ++i) {
        std::cout << tokens[i].context.toS() << " " << tokens[i].toS() << std::endl;
    }
    std::cout << std::endl;
}



int main(int argc, char** argv) {
    rx_test_suite();
    return 0;

    if (argc != 2) {
        printf("Invalid amount of arguments (%d). Supply a single filename.\n", argc-1);
        return EXIT_FAILURE;
    }

    Seq<Token> tokens = get_tokens_from_file(argv[1]);
    // print_tokens(tokens);
    std::cout << "Parsed " << tokens.size() << " tokens." << std::endl;

// // THIS PERFECTLY REPRESENTS WHY HERE STRINGS IS A GOOD IDEA
//     Token_context tc;
//     // tc.file = __FILE__;
//     tc.line = 417;
//     tc.position = 57;

//     Seq<Token> tokens = get_tokens_from_string("asd\n\
// typeof :: inline fn(t : $T) -> type\n\
// #modify\n\
// {\n\
//     while (T == any) {\n\
//         T = T.type; /* COMMENT! // */\n\
//     */ }\n\
// }\n\
// {\n\
//     return /* COMMENT! */ T;\n\
// };",
//     tc);
//     print_tokens(tokens);

}

#endif


