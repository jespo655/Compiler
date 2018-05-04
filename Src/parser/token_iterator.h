#pragma once

#include "token.h"
#include "../utilities/error_handler.h"
#include "../utilities/assert.h"
#include "../utilities/sequence.h"

#include <memory>
#include <string>
#include <vector>

/*
A Token_iterator holds a reference to a list of Tokens and iterates through it.
This list has to be stored elsewhere, deallocate as long as the Token_iterator is alive.

Normal usage is one of the following:
* Get the next token with eat_token(), then check if it's valid. If not, log an error.
* Get the next token with expect(...). This logs an error if the token wasn't what was expected.
    Check if the token is valid either manually, or through expect_failed()
* Get a future token with look_ahead(int) to check several tokens in a row. Then use eat_tokens() to eat them all at once.
*/
struct Token_iterator
{
    const Seq<Token>& tokens; // should always end with an EOF-token
    int current_index = 0;
    bool error = false;

    Token_iterator(const Seq<Token>& tokens, int current_index=0) : tokens{tokens}, current_index{current_index}
    {
        ASSERT(tokens.size > 0); // not empty
        ASSERT(tokens[tokens.size-1].is_eof()); // last token is eof token
    }

    // iterator interface: prefix* and prefix++ operators
    const Token& operator*() { return current_token(); }
    const Token_iterator& operator++() { eat_token(); return *this; }
    Token const * operator->() { return &current_token(); }
    const Token& operator[](int index) { return look_ahead(index-current_index); }


    // Returns the current token.
    const Token& current_token()
    {
        error = false;
        return tokens[current_index];
    }

    bool compare(Token_type type, const std::string& token)
    {
        const Token& t = current_token();
        return t.type == type && t.token == token;
    }

    // Returns the token at a specific index.
    // Expects the index n to be inside the bounds of the token vector
    const Token& look_at(int n) {
        if (n < 0 || n >= tokens.size) {
            error = true;
            return tokens[tokens.size-1];
        }
        error = false;
        return tokens[n];
    }

    // Returns a token n steps ahead.
    const Token& look_ahead(int n) { return look_at(current_index+n); }


    // Returns a token n steps back.
    const Token& look_back(int n) { return look_at(current_index-n); }


    // Returns the current token and increments the current_index.
    const Token& eat_token()
    {
        const Token& t = current_token(); // sets error to false
        if (current_index < tokens.size-1)
            current_index++;
        return t;
    }


    // Increments the current_index by n.
    void eat_tokens(int n)
    {
        ASSERT(n > 0);
        if (current_index+n >= tokens.size) current_index = tokens.size;
        else current_index += n;
        error = false;
    }


    // Returns the current token and increments the current_index.
    // If the token type didn't match the expected type, also logs an error.
    const Token& expect(const Token_type& expected_type)
    {
        const Token& t = eat_token(); // sets error to false
        if (t.type != expected_type) {
            log_error("Expected token of type "+toS(expected_type)+", but found type "+toS(t.type), t.context);
            error = true;
        }
        return t;
    }


    // Returns the current token and increments the current_index.
    // If the token didn't match the expectation, also logs an error.
    const Token& expect(const Token_type& expected_type, std::string expected_token)
    {
        const Token& t = eat_token(); // sets error to false
        if (t.type != expected_type || t.token != expected_token) {
            log_error("Expected token \""+expected_token+"\" ("+toS(expected_type)+"), but found \""+t.token+"\" ("+toS(t.type)+")", t.context);
            error = true;
        }
        return t;
    }

    const Token& expect_current(const Token_type& expected_type)
    {
        const Token& t = current_token(); // sets error to false
        if (t.type != expected_type) {
            log_error("Expected token of type "+toS(expected_type)+", but found type "+toS(t.type), t.context);
            error = true;
        }
        return t;
    }

    const Token& expect_current(const Token_type& expected_type, std::string expected_token)
    {
        const Token& t = current_token(); // sets error to false
        if (t.type != expected_type || t.token != expected_token) {
            log_error("Expected token \""+expected_token+"\" ("+toS(expected_type)+"), but found \""+t.token+"\" ("+toS(t.type)+")", t.context);
            error = true;
        }
        return t;
    }

    const Token& expect_end_of_statement()
    {
        const Token& t = eat_token(); // sets error to false
        if (t.type != Token_type::SYMBOL || t.token != ";") {
            log_error("Missing ';' at the end of statement: expected \";\" before \""+t.token+"\"", t.context);
            error = true;
        }
        return t;
    }

    // Returns true if the last expect call failed.
    bool expect_failed() { return error; }

    // ASSERTs that the current token is next, then eat it
    // Used like expect, but gives compiler runtime error instead of a logged compile error.
    const Token& assert(const Token_type& expected_type, std::string expected_token="")
    {
        ASSERT(!error);
        ASSERT((*this)->type == expected_type && expected_token=="" || (*this)->token == expected_token);
        return eat_token();
    }

    const Token& assert_current(const Token_type& expected_type, std::string expected_token="")
    {
        ASSERT(!error);
        ASSERT((*this)->type == expected_type && expected_token=="" || (*this)->token == expected_token);
        return current_token();
    }


    // If error, logs an appropriate error and returns -1
    int find_matching_token(int index, const Token_type& expected_closing_type, const std::string& expected_closing_token, const std::string& range_name, const std::string& error_string, bool forward=true, bool log_errors=true)
    {
        const Token& start_token = look_at(index);
        ASSERT(!start_token.is_eof());

        int step = forward ? 1 : -1;

        while(true) {
            const Token& t = look_at(index);

            if (t.is_eof()) {
                if (log_errors) {
                    log_error("Missing \""+expected_closing_token+"\" at end of file",t.context);
                    if (forward) add_note("In "+range_name+" that started here: ",start_token.context);
                    else add_note("While searching backwards from "+range_name+" that started here: ",start_token.context);
                }
                error = true;
                return -1;
            }

            if (t.type == expected_closing_type && t.token == expected_closing_token) {
                error = false;
                return index; // done!
            }

            if (t.type == Token_type::SYMBOL) {

                if (forward) {
                    if      (t.token == "(") index = find_matching_paren(index);
                    else if (t.token == "[") index = find_matching_bracket(index);
                    else if (t.token == "{") index = find_matching_brace(index);

                    else if (t.token == ")" || t.token == "]" || t.token == "}") {
                        if (log_errors) {
                            log_error(error_string+": expected \""+expected_closing_token+"\" before \""+t.token+"\"",t.context);
                            add_note("In "+range_name+" that started here: ",start_token.context);
                        }
                        error = true;
                        return -1;
                    }
                } else {
                    if      (t.token == ")") index = find_matching_paren(index);
                    else if (t.token == "]") index = find_matching_bracket(index);
                    else if (t.token == "}") index = find_matching_brace(index);

                    else if (t.token == "(" || t.token == "[" || t.token == "{") {
                        if (log_errors) {
                            log_error(error_string+": expected \""+expected_closing_token+"\" before \""+t.token+"\"",t.context);
                            add_note("While searching backwards from "+range_name+" that started here: ",start_token.context);
                        }
                        error = true;
                        return -1;
                    }
                }

                if (index == -1) return -1;
            }
            index += step;
        }
    }

    int find_matching_paren(int index=-1)
    {
        if (index == -1) index = current_index;
        ASSERT(index >= 0 && index < tokens.size && tokens[index].type == Token_type::SYMBOL);
        // ASSERT(tokens[index].token == "(" || tokens[index].token == ")");
        if (tokens[index].token == ")")
             return find_matching_token(index-1, Token_type::SYMBOL, "(","paren","Mismatched paren", false); // search backwards
        else return find_matching_token(index+1, Token_type::SYMBOL, ")","paren","Mismatched paren");
    }

    int find_matching_bracket(int index=-1)
    {
        if (index == -1) index = current_index;
        ASSERT(index >= 0 && index < tokens.size && tokens[index].type == Token_type::SYMBOL);
        // ASSERT(tokens[index].token == "[" || tokens[index].token == "]");
        if (tokens[index].token == "]")
             return find_matching_token(index-1, Token_type::SYMBOL, "[","bracket","Mismatched bracket", false); // search backwards
        else return find_matching_token(index+1, Token_type::SYMBOL, "]","bracket","Mismatched bracket");
    }

    int find_matching_brace(int index=-1)
    {
        if (index == -1) index = current_index;
        ASSERT(index >= 0 && index < tokens.size && tokens[index].type == Token_type::SYMBOL);
        // ASSERT(tokens[index].token == "{" || tokens[index].token == "}");
        if (tokens[index].token == "}")
             return find_matching_token(index-1, Token_type::SYMBOL, "{","brace","Mismatched brace", false); // search backwards
        else return find_matching_token(index+1, Token_type::SYMBOL, "}","brace","Mismatched brace");
    }

    int find_matching_semicolon(int index=-1)
    {
        if (index == -1) index = current_index;
        ASSERT(index >= 0 && index < tokens.size);
        return find_matching_token(index, Token_type::SYMBOL, ";", "statement","Missing ';' at the end of statement");
    }



};






