#pragma once

#include "../abstx/scope.h"
#include "../abstx/function.h"
#include "../abstx/using.h"
#include "token.h"

#include <memory>
#include <string>
#include <vector>

struct Parsed_scope : Scope
{
    std::vector<std::shared_ptr<Using_statement>> using_statements;
    std::vector<std::shared_ptr<Anonymous_scope>> anonymous_scopes;
};

struct Global_scope : Parsed_scope
{
    std::string file_name;
    const std::vector<Token> tokens; // should be treated as const

    std::vector<std::shared_ptr<Function_call_statement>> run_statements;

    Global_scope(const std::vector<Token>& tokens) : tokens{tokens} {}
};

std::shared_ptr<Global_scope> parse_file(const std::string& file, const std::string& name = ""); // default name is the file name
std::shared_ptr<Global_scope> parse_string(const std::string& string, const std::string& name = ""); // FIXME: add string context
std::shared_ptr<Global_scope> parse_tokens(const std::vector<Token>& tokens, const std::string& name = "");




/**
förarbete:
    Global_scope skapas och läggs in i en map file_name -> global_scope
    UTF8 character stream skaps av input (läses från fil eller string)
    En lista av tokens skapas och sparas i global_scope

pass 1) listan av tokens gås igenom, en lista av statement ställs upp

    identifiers läggs till i scope
        varje identifier måste veta i vilket statement den blev deklarerad (owner?)
    en lista på using statements sparas
    en lista på #run statements sparas
    en lista på under-scopes sparas

    parentes mismatch -> FATAL_ERROR, avbryter kompilering
    andra errors -> tolka som "undefined statement" som får resolvas senare, gå till nästa ';' i samma paren-nivå som början av statement

    slut av pass 1:
    markera som partially parsed
    importera saker med using (partially parsa dem först)
    partially parsa alla under-scopes

pass 2) #runs gås igenom
    varje statement fully resolvas först, sen körs
    en funktion är fully resolvad om alla identifiers i den är fully resolvade
        och om alla funktioner som anropas är åtminstone partially resolved

pass 3) output
    funktioner gås igenom från entry point
    följ funktionsanrop från entry point och ställ upp en lista på alla saker som används
    alla funktioner och datatyper som används fördeklareras
    sen skriv ut definitioner av alla datatyper
    sen skriv ut alla funktioner som används

*/




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
    const std::vector<Token>& tokens; // should always end with an EOF-token
    int current_index = 0;
    bool error = false;

    Token_iterator(const std::vector<Token>& tokens, int current_index=0) : tokens{tokens}, current_index{current_index}
    {
        ASSERT(!tokens.empty()); // not empty
        ASSERT(tokens[tokens.size()-1].is_eof()); // last token is eof token
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


    // Returns a token n steps ahead.
    // Expects the current_index to be inside the bounds of the token vector
    const Token& look_ahead(int n)
    {
        if (current_index+n < 0 || current_index+n >= tokens.size()) {
            error = true;
            return tokens[tokens.size()-1]; // should be EOF token
        }
        error = false;
        return tokens[current_index+n];
    }


    // Returns a token n steps back.
    const Token& look_back(int n) { return look_ahead(-n); }


    // Returns the current token and increments the current_index.
    const Token& eat_token()
    {
        const Token& t = current_token(); // sets error to false
        if (current_index < tokens.size()-1)
            current_index++;
        return t;
    }


    // Increments the current_index by n.
    void eat_tokens(int n)
    {
        ASSERT(n > 0);
        if (current_index+n >= tokens.size()) current_index = tokens.size();
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


    // Returns true if the last expect call failed.
    bool expect_failed() { return error; }


    // If error, logs an appropriate error and returns -1
    int find_matching_token(int index, const std::string& expected_closing_token, const std::string& range_name, const std::string& error_string, bool forward=true)
    {
        const Token& start_token = look_ahead(index-current_index);
        ASSERT(!start_token.is_eof());

        int step = forward ? 1 : -1;

        while(true) {
            const Token& t = look_ahead(index-current_index);

            if (t.is_eof()) {
                log_error("Missing \""+expected_closing_token+"\" at end of file",t.context);
                if (forward) add_note("In "+range_name+" that started here: ",start_token.context);
                else add_note("While searching backwards from "+range_name+" that started here: ",start_token.context);
                error = true;
                return -1;
            }
            if (t.type == Token_type::SYMBOL) {

                if (t.token == expected_closing_token) {
                    error = false;
                    return index; // done!
                }

                if (forward) {
                    if      (t.token == "(") index = find_matching_paren(index);
                    else if (t.token == "[") index = find_matching_bracket(index);
                    else if (t.token == "{") index = find_matching_brace(index);

                    else if (t.token == ")" || t.token == "]" || t.token == "}") {
                        log_error(error_string+": expected \""+expected_closing_token+"\" before \""+t.token+"\"",t.context);
                        add_note("In "+range_name+" that started here: ",start_token.context);
                        error = true;
                        return -1;
                    }
                } else {
                    if      (t.token == ")") index = find_matching_paren(index);
                    else if (t.token == "]") index = find_matching_bracket(index);
                    else if (t.token == "}") index = find_matching_brace(index);

                    else if (t.token == "(" || t.token == "[" || t.token == "{") {
                        log_error(error_string+": expected \""+expected_closing_token+"\" before \""+t.token+"\"",t.context);
                        add_note("While searching backwards from "+range_name+" that started here: ",start_token.context);
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
        ASSERT(index >= 0 && index < tokens.size() && tokens[index].type == Token_type::SYMBOL);
        ASSERT(tokens[index].token == "(" || tokens[index].token == ")");
        if (tokens[index].token == ")") return find_matching_token(index-1,"(","paren","Mismatched paren", false); // search backwards
        return find_matching_token(index+1,")","paren","Mismatched paren");
    }

    int find_matching_bracket(int index=-1)
    {
        if (index == -1) index = current_index;
        ASSERT(index >= 0 && index < tokens.size() && tokens[index].type == Token_type::SYMBOL);
        ASSERT(tokens[index].token == "[" || tokens[index].token == "]");
        if (tokens[index].token == "]") return find_matching_token(index-1,"[","bracket","Mismatched bracket", false); // search backwards
        return find_matching_token(index+1,"]","bracket","Mismatched bracket");
    }

    int find_matching_brace(int index=-1)
    {
        if (index == -1) index = current_index;
        ASSERT(index >= 0 && index < tokens.size() && tokens[index].type == Token_type::SYMBOL);
        ASSERT(tokens[index].token == "{" || tokens[index].token == "}");
        if (tokens[index].token == "}") return find_matching_token(index-1,"{","brace","Mismatched brace", false); // search backwards
        return find_matching_token(index+1,"}","brace","Mismatched brace");
    }

    int find_matching_semicolon(int index=-1)
    {
        if (index == -1) index = current_index;
        ASSERT(index >= 0 && index < tokens.size());
        return find_matching_token(index, ";", "statement","Missing ';' at the end of statement");
    }


};




































/*
Pass 1: Partial parsing

Alla filer gås igenom och dess static scope parsas partiellt


Varje statement gås igenom mha parentes-checkar (allt som nämns här är endast utanför parenteser)

TILLÅTET I STATIC SCOPE:

Hittar ':' token: declaration
    Allting innan ':' måste vara nydeklarerade identifiers (detta kan kollas)
    Deklarerade identifiers läggs till i local_scope->identifiers
    Inget efter ':' parsas.
    -> gå till nästa ';'

using statement (using "file.h"; using test;)
    Lägg till i en lista av using-statements, resolva dem sist (måste fully resolvas innan scope är partially resolvat)
    Om string literal -> inkludera den filen (kolla först i listan av kompilerade filer)
        öppna den filen, skapa nytt workspace, försök partially resolva det, lägg till global scope i using-listan
    Om identifier -> försök partially resolva den. Om scope identifier, lägg till i using-listan. Om något annat -> type error
    -> gå till ';'

statement som börjar med '{'
    Anonymous scope
    Parsa partiellt sist (efter using)
    -> gå till matchande '}'

statement som börjar med #run
    lägg till i en lista av #run (i workspace)
    lägg inte till som statement i static scope
    kör sist (efter using och efter anonymous scopes, de kan också innehålla #runs) (compile time execution)
    (om det inte finns några #run, avsluta kompileringen med tom output. Entry point måste sättas någonstans.)
    -> gå till ';'




INTE TILLÅTET I STATIC SCOPE: (detta skulle ge SYNTAX_ERROR)

Inget av detta hittas, men expr slutar med en parentes '()' och sen direkt ';': Function call
    Inget parsas.
    -> gå till ';'

return statement
    Inget parsas.
    -> gå till ';'

if, elsif, else, then, for, while
    Kräv följande '()' (där det är relevant) och '{}'-scope
    -> gå till matchande '}''

Hittar '=' token: assignment
    Inget parsas.
    -> gå till nästa ';'

Inget av detta, men följer operator mönster:
    expr @ expr; -> infix_operator
    expr @; -> suffix_operator
    @ expr; -> prefix_operator
        (där @ är en identifier eller symbol token)
    Ger syntax error i pass 1 ("unable to parse statement"), men borde kunna tolkas och parsas fullt direkt i pass 2 och 3
    -> gå till ';'

Vad som helst annat:
    Syntax error
    -> gå till ';'




Pass 2: #run

Alla #run från global scope körs

Ett statement i taget parsas helt
Sätt upp dependencies om något inte går att parsa, och parsa dem först.
    Dependencies behövs endast för att undvika cykler
    Om en cyklisk dependency hittas -> log error (CYCLIC_DEPENDENCY)

Interpretera koden och kör den compile time, en funktion i taget.
    Runtime error -> fånga och log error (COMPILE_TIME_ERROR)

När alla #runs är klara, se till att output entry point är satt
    Annars -> log error (COMPILE_TIME_ERROR)




Pass 3: output

Utgå från output entry point
Kör som i pass 2, men istället för att evaluera allt, översätt till c-kod och skriv till fil
Fully resolva ett statement i taget och översätt. Allt borde vara mycket likt pass 2.

*/





/*
The parser builds an abstract syntax tree from a set of tokens
*/



/*
Problem:

for incremental compilation, each statement has to know about its own list of tokens (unless finalized)

    Each abstx has a start token pointer
    To iterate through tokens, construct a token iterator with this pointer

    The list of tokens end with eof token
    The iterator cannot go past this token

    Make sure that the list of tokens doesn't deallocate or reallocate
        it cannot change size
        look out for possible reallocations / heap cleanup (does that even happen in c++?)
*/



/*

Partial parsing paren mismatch recovery:

Gå tillbaka till början av statement
Försök fully resolva statement
Detta kan ge bra felmeddelanden med sammanhang:

ex:
    int a = ); // "unexpected ')' in rhs of assignment"

Om det går att städa upp, gör det och skicka tillbaka SYNTAX_ERROR. Om inte, returnera FATAL_ERROR.




i c++:

    {} matchar alltid, oberoende av potentiella fel mellan
    vid en extra ')': gå till nästa ';' eller '}', men fortsätt matcha scopes {}

*/