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






// Partial parse of declaration statement
Parsing_status partially_parse(Token_iterator& it, std::shared_ptr<Declaration_statement>& statement, int declaration_index, std::shared_ptr<Scope> parent_scope)
{
    ASSERT (!it.error);
    ASSERT (it[declaration_index].type == Token_type::SYMBOL && it[declaration_index].token == ":");
    ASSERT (it.current_index < declaration_index);

    statement = std::shared_ptr<Declaration_statement>{new Declaration_statement()};

    bool first = true;

    while(!it.error && it.current_index < declaration_index) {

        if (!first) it.expect(Token_type::SYMBOL, ",");
        if (it.error) break;

        const Token& identifier_token = it.expect(Token_type::IDENTIFIER);

        if (!it.error) {
            auto id = std::shared_ptr<Identifier>(new Identifier());
            id->name = identifier_token.token;
            id->context = identifier_token.context;
            id->owner = statement;
            id->status = Parsing_status::PARTIALLY_PARSED;
            id->start_token_index = it.current_index-1;

            statement->identifiers.push_back(id);
            parent_scope->identifiers[id->name] = id;
        }

        if (it.current_index == declaration_index) break;

        first = false;
    }

    if (it.error) statement->status = Parsing_status::FATAL_ERROR;
    else statement->status = Parsing_status::PARTIALLY_PARSED;

    return statement->status;
}





// Partial parse of for statement
Parsing_status partially_parse(Token_iterator& it, std::shared_ptr<For_statement>& statement)
{
    ASSERT (it->type != Token_type::STRING && it->token == "for");
    ASSERT (!it.error);

    statement = std::shared_ptr<For_statement>{new For_statement()};

    it.eat_token(); // eat the "for" token

    it.expect(Token_type::SYMBOL, "(");
    if (!it.error) it.current_index = it.find_matching_paren(it.current_index-1) + 1; // go back to the previous "(" and search from there

    statement->scope = std::shared_ptr<Scope>{new Scope()};
    statement->scope->owner = statement;
    statement->scope->start_token_index = it.current_index;

    if (!it.error) it.expect(Token_type::SYMBOL, "{");
    if (!it.error) it.current_index = it.find_matching_brace(it.current_index-1) + 1; // same as above

    if (it.error) statement->status = Parsing_status::FATAL_ERROR;
    else statement->status = Parsing_status::PARTIALLY_PARSED;

    return statement->status;
}




// Partial parse of while statement
Parsing_status partially_parse(Token_iterator& it, std::shared_ptr<While_statement>& statement)
{
    ASSERT (it->type != Token_type::STRING && it->token == "while");
    ASSERT (!it.error);

    statement = std::shared_ptr<While_statement>{new While_statement()};

    it.eat_token(); // eat the "while" token

    it.expect(Token_type::SYMBOL, "(");
    if (!it.error) it.current_index = it.find_matching_paren(it.current_index-1) + 1; // go back to the previous "(" and search from there

    statement->scope = std::shared_ptr<Scope>{new Scope()};
    statement->scope->owner = statement;
    statement->scope->start_token_index = it.current_index;

    if (!it.error) it.expect(Token_type::SYMBOL, "{");
    if (!it.error) it.current_index = it.find_matching_brace(it.current_index-1) + 1; // same as above

    if (it.error) statement->status = Parsing_status::FATAL_ERROR;
    else statement->status = Parsing_status::PARTIALLY_PARSED;

    return statement->status;
}




// Partial parse of if statement
Parsing_status partially_parse(Token_iterator& it, std::shared_ptr<If_statement>& statement)
{
    ASSERT (it->type != Token_type::STRING && it->token == "if");
    ASSERT (!it.error);

    statement = std::shared_ptr<If_statement>{new If_statement()};

    // read any number of conditional scopes (if, elsif, elsif, ...)
    do {
        it.eat_token(); // eat the "if"/"elsif" token

        auto cs = std::shared_ptr<Conditional_scope>{new Conditional_scope()};
        cs->owner = statement;
        cs->start_token_index = it.current_index;
        statement->conditional_scopes.push_back(cs);

        it.expect(Token_type::SYMBOL, "(");
        if (!it.error) it.current_index = it.find_matching_paren(it.current_index-1) + 1; // go back to the previous "(" and search from there

        cs->scope = std::shared_ptr<Scope>{new Scope()};
        cs->scope->owner = cs;
        cs->scope->start_token_index = it.current_index;

        if (!it.error) it.expect(Token_type::SYMBOL, "{");
        if (!it.error) it.current_index = it.find_matching_brace(it.current_index-1) + 1; // same as above

    } while (!it.error && it->type != Token_type::STRING && it->token == "elsif");

    if (!it.error && it->type != Token_type::STRING && it->token == "else") {

        it.eat_token(); // eat the "else" token

        statement->else_scope = std::shared_ptr<Scope>{new Scope()};
        statement->else_scope->owner = statement;
        statement->else_scope->start_token_index = it.current_index;

        if (!it.error) it.expect(Token_type::SYMBOL, "{");
        if (!it.error) it.current_index = it.find_matching_brace(it.current_index-1) + 1; // same as above
    }

    if (!it.error && it->type != Token_type::STRING && it->token == "then") {

        it.eat_token(); // eat the "then" token

        statement->then_scope = std::shared_ptr<Scope>{new Scope()};
        statement->then_scope->owner = statement;
        statement->then_scope->start_token_index = it.current_index;

        if (!it.error) it.expect(Token_type::SYMBOL, "{");
        if (!it.error) it.current_index = it.find_matching_brace(it.current_index-1) + 1; // same as above
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
    }

    else if (start_token.type != Token_type::STRING && start_token.token == "while") {
        if (!allow_dynamic) log_error("while statement not allowed in static scope", start_token.context); // FIXME: proper error message
        auto while_s = std::shared_ptr<While_statement>{new While_statement()};
        partially_parse(it, while_s);
        statement = std::static_pointer_cast<Statement>(while_s);
    }

    else if (start_token.type != Token_type::STRING && start_token.token == "return") {
        if (!allow_dynamic) log_error("Return statement not allowed in static scope", start_token.context); // FIXME: proper error message
        statement = std::shared_ptr<Statement>{new Return_statement()};
        it.current_index = it.find_matching_semicolon(it.current_index) + 1;
        if (it.error) statement->status = Parsing_status::FATAL_ERROR;
        else statement->status = Parsing_status::PARTIALLY_PARSED;
    }

    else while (!it.error && !it->is_eof()) {

        // look for ':' (declaration), '=' (assignment) and ';' (unknown statement)
        // the first such matching symbol found determines the type of statement.
        // If an unknown statement ends with ");", then it's a function call

        const Token& t = it.eat_token();

        if (t.type == Token_type::SYMBOL) {

            if (t.token == ":") {
                statement = std::shared_ptr<Statement>(new Declaration_statement());
                auto decl_s = std::shared_ptr<Declaration_statement>{new Declaration_statement()};
                int decl_index = it.current_index-1; // the ':' token
                it.current_index = start_index;
                partially_parse(it, decl_s, decl_index, parent_scope);
                statement = std::static_pointer_cast<Statement>(decl_s);
                break;
            }

            if (t.token == "=") {
                statement = std::shared_ptr<Statement>(new Assignment_statement());
                if (go_to_next_statement) it.current_index = it.find_matching_semicolon(it.current_index) + 1;
                if (it.error) statement->status = Parsing_status::FATAL_ERROR;
                else statement->status = Parsing_status::PARTIALLY_PARSED;
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