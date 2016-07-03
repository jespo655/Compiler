#include "parser.h"
#include "lexer.h"
#include "parsing_status.h"

#include "../abstx/scope.h"
#include "../abstx/using.h"
#include "../abstx/statement.h"
#include "../abstx/if.h"
#include "../abstx/for.h"
#include "../abstx/while.h"
#include "../abstx/function.h"
#include "../abstx/declaration.h"
#include "../abstx/assignment.h"
#include "../abstx/return.h"

std::shared_ptr<Statement> examine_statement(Token_iterator& it, std::shared_ptr<Parsed_scope> parent_scope, bool allow_dynamic=false, bool go_to_next_statement=true);
Parsing_status partially_parse(Token_iterator& it, std::shared_ptr<If_statement>& statement);











std::shared_ptr<Global_scope> parse_file(const std::string& file, const std::string& name) // default name is the file name
{
    return parse_tokens(get_tokens_from_file(file), name);
}

std::shared_ptr<Global_scope> parse_string(const std::string& string, const std::string& name) // FIXME: add string context
{
    return parse_tokens(get_tokens_from_string(string), name);
}

std::shared_ptr<Global_scope> parse_tokens(const std::vector<Token>& tokens, const std::string& name)
{
    std::shared_ptr<Global_scope> global_scope{new Global_scope(tokens)};
    Token_iterator it = Token_iterator(tokens);

    // do {

    //     std::shared_ptr<Statement> s = examine_statement(it, global_scope);
    //     if (s.status == Parsing_status::FATAL_ERROR) {
    //         global_scope->status = FATAL_ERROR;
    //         return global_scope;
    //     }

    //     global_scope->statements.push_back(s);

    // }


    return global_scope;
}

















Parsing_status partially_parse(Token_iterator& it, std::shared_ptr<For_statement>& statement)
{
    return Parsing_status::NOT_PARSED;
}




Parsing_status partially_parse(Token_iterator& it, std::shared_ptr<If_statement>& statement)
{
    ASSERT (it->type != Token_type::STRING && it->token == "if");
    it.eat_token(); // eat the "if" token

    statement = std::shared_ptr<If_statement>{new If_statement()};
    it.expect(Token_type::SYMBOL, "(");
    if (!it.error) it.current_index = it.find_matching_paren(it.current_index-1) + 1; // go back to the previous "(" and search from there
    if (!it.error) it.expect(Token_type::SYMBOL, "{");
    if (!it.error) it.current_index = it.find_matching_brace(it.current_index-1) + 1; // same as above

    while (!it.error && it->type != Token_type::STRING && it->token == "elsif") {
        // FIXME: also add the scopes and other possibly interesting information to the if statement
        it.eat_token();
        it.expect(Token_type::SYMBOL, "(");
        if (!it.error) it.current_index = it.find_matching_paren(it.current_index-1) + 1;
        if (!it.error) it.expect(Token_type::SYMBOL, "{");
        if (!it.error) it.current_index = it.find_matching_brace(it.current_index-1) + 1;
    }
    if (!it.error && it->type != Token_type::STRING && it->token == "else") {
        it.eat_token();
        if (!it.error) it.expect(Token_type::SYMBOL, "{");
        if (!it.error) it.current_index = it.find_matching_brace(it.current_index-1) + 1;
    }
    if (!it.error && it->type != Token_type::STRING && it->token == "then") {
        it.eat_token();
        if (!it.error) it.expect(Token_type::SYMBOL, "{");
        if (!it.error) it.current_index = it.find_matching_brace(it.current_index-1) + 1;
    }

    if (it.error) statement->status = Parsing_status::FATAL_ERROR;
    else statement->status = Parsing_status::PARTIALLY_PARSED;

    return statement->status;
}












// if go_to_next_statement is false, the iterator will be reset to the beginning of the statement
// returns nullptr if no more statements could be read.
std::shared_ptr<Statement> examine_statement(Token_iterator& it, std::shared_ptr<Parsed_scope> parent_scope, bool allow_dynamic, bool go_to_next_statement)
{
    int start_index = it.current_index;
    Token_context context = it->context;
    std::shared_ptr<Statement> statement{nullptr};

    const Token& start_token = it.current_token();

    if (start_token.is_eof()) {
        // no error, just return nullptr as expected
        return nullptr;

    } else if (start_token.type == Token_type::SYMBOL && start_token.token == ";") {
        // empty statement, give warning and ignore
        log_warning("Additional ';' found", start_token.context);
        return examine_statement(it, parent_scope, allow_dynamic, go_to_next_statement);

    } else if (start_token.type != Token_type::STRING && start_token.token == "using") {
        auto us = std::shared_ptr<Using_statement>{new Using_statement()};
        parent_scope->using_statements.push_back(us);
        statement = std::static_pointer_cast<Statement>(us);
        it.current_index = it.find_matching_semicolon(it.current_index) + 1;

    } else if (start_token.type == Token_type::SYMBOL && start_token.token == "{") {
        auto scope = std::shared_ptr<Anonymous_scope>{new Anonymous_scope()};
        parent_scope->anonymous_scopes.push_back(scope);
        it.current_index = it.find_matching_brace();
        statement = std::static_pointer_cast<Statement>(scope);

    }

    else if (start_token.type != Token_type::STRING && start_token.token == "if") {
        if (!allow_dynamic) log_error("if statement not allowed in static scope", start_token.context); // FIXME: proper error message
        auto if_s = std::shared_ptr<If_statement>{new If_statement()};
        partially_parse(it, if_s);
        statement = std::static_pointer_cast<Statement>(if_s);
    }

    else if (start_token.type != Token_type::STRING && start_token.token == "for") {
        if (!allow_dynamic) log_error("for statement not allowed in static scope", start_token.context); // FIXME: proper error message
        auto for_s = std::shared_ptr<For_statement>{new For_statement()};
        partially_parse(it, for_s);
        statement = std::static_pointer_cast<Statement>(for_s);



        // statement = std::shared_ptr<Statement>{new For_statement()};
        // statement_ends_with_semicolon = false;
        // it.eat_token(); // eat the "for" token
        // it.expect(Token_type::SYMBOL, "(");
        // if (!it.error) it.current_index = it.find_matching_paren(it.current_index-1) + 1; // go back to the previous "(" and search from there
        // if (!it.error) it.expect(Token_type::SYMBOL, "{");
        // if (!it.error) it.current_index = it.find_matching_brace(it.current_index-1) + 1; // go back to the previous "{" and search from there
    }

    else if (start_token.type != Token_type::STRING && start_token.token == "while") {
        if (!allow_dynamic) log_error("while statement not allowed in static scope", start_token.context); // FIXME: proper error message
        statement = std::shared_ptr<Statement>{new While_statement()};
        it.eat_token(); // eat the "while" token
        it.expect(Token_type::SYMBOL, "(");
        if (!it.error) it.current_index = it.find_matching_paren(it.current_index-1) + 1; // go back to the previous "(" and search from there
        if (!it.error) it.expect(Token_type::SYMBOL, "{");
        if (!it.error) it.current_index = it.find_matching_brace(it.current_index-1) + 1; // go back to the previous "{" and search from there
    }

    else if (start_token.type != Token_type::STRING && start_token.token == "return") {
        if (!allow_dynamic) log_error("Return statement not allowed in static scope", start_token.context); // FIXME: proper error message
        statement = std::shared_ptr<Statement>{new Return_statement()};
        it.current_index = it.find_matching_semicolon(it.current_index) + 1;
    }

    else while (!it.error && !it->is_eof()) {

        // look for ':' (declaration), '=' (assignment) and ';' (unknown statement)
        // the first such matching symbol found determines the type of statement.
        // If an unknown statement ends with ");", then it's a function call

        const Token& t = it.eat_token();

        if (t.type == Token_type::SYMBOL) {

            if (t.token == ":") {
                statement = std::shared_ptr<Statement>(new Declaration_statement());
                ASSERT(false, "decl assignment partial parsing nyi"); // FIXME: add the declared identifiers to the scope
                if (go_to_next_statement) it.current_index = it.find_matching_semicolon(it.current_index) + 1;
                break;
            }

            if (t.token == "=") {
                statement = std::shared_ptr<Statement>(new Assignment_statement());
                if (go_to_next_statement) it.current_index = it.find_matching_semicolon(it.current_index) + 1;
                break;
            }

            if (t.token == ";") {
                const Token& lb2 = it.look_back(2);
                if (lb2.type == Token_type::SYMBOL && lb2.token == ")") {
                    auto fc = std::shared_ptr<Function_call>(new Function_call());
                    auto fc_statement = std::shared_ptr<Function_call_statement>{new Function_call_statement()};
                    fc_statement->function_call = fc;
                    fc->owner = fc_statement;
                    statement = std::static_pointer_cast<Statement>(fc_statement);

                    if (start_token.type == Token_type::COMPILER_COMMAND && start_token.token == "#run") {
                        // FIXME: ensure that #run has the correct case (#RUN / #run)
                        fc->compile_time = true;
                        fc->start_token_index = start_index+1;
                        fc->context = it[start_index+1].context;
                        parent_scope->run_statements.push_back(fc_statement);
                    } else {
                        fc->compile_time = false;
                        fc->start_token_index = start_index;
                        fc->context = context;
                    }

                } else {
                    statement = std::shared_ptr<Statement>(new Unknown_statement());
                }
                break;
            }

            if (t.token == "(") it.current_index = it.find_matching_paren(it.current_index-1) + 1;  // go back to the previous "(" and search from there
            else if (t.token == "[") it.current_index = it.find_matching_bracket(it.current_index-1) + 1;
            else if (t.token == "{") it.current_index = it.find_matching_brace(it.current_index-1) + 1;

            else if (t.token == ")" || t.token == "]" || t.token == "}") {
                log_error("Unexpected token "+t.token+" in unresolved statement.", t.context);
                statement = std::shared_ptr<Statement>(new Unknown_statement());
                statement->status = Parsing_status::FATAL_ERROR;
                it.error = true;
                break;
            }
        }
    }

    ASSERT(statement != nullptr)

    statement->start_token_index = start_index;
    statement->owner = parent_scope;
    statement->context = context;

    ASSERT(!it.error || statement->status == Parsing_status::FATAL_ERROR);

    if (!go_to_next_statement) {
        it.current_index = start_index;
    }

    return statement;
}








/*


Parsing_status Global_scope::parse_partially()
{
// pass 1) listan av tokens gås igenom, en lista av statement ställs upp

//     identifiers läggs till i scope
//         varje identifier måste veta i vilket statement den blev deklarerad (owner?)
//     en lista på using statements sparas
//     en lista på #run statements sparas
//     en lista på under-scopes sparas

//     parentes mismatch -> FATAL_ERROR, avbryter kompilering
//     andra errors -> tolka som "undefined statement" som får resolvas senare, gå till nästa ';' i samma paren-nivå som början av statement

//     slut av pass 1:
//     markera som partially parsed
//     importera saker med using (partially parsa dem först)
//     partially parsa alla under-scopes

    Token_iterator it = Token_iterator(tokens);




    return Parsing_status::NOT_PARSED;
}




*/