#include "parser.h"
// all types of statements are needed for static casts
#include "../abstx/statement.h"
#include "../abstx/declaration.h"
#include "../abstx/assignment.h"
#include "../abstx/if.h"
#include "../abstx/for.h"
#include "../abstx/while.h"
#include "../abstx/using.h"
#include "../abstx/return.h"
#include "../abstx/defer.h"
#include "../abstx/scope.h"


// --------------------------------------------------------------------------------------------------------------------
//
//          Statement examination
//          Used to determine which type of statement to read
//          Also used to verify that the general code structure is valid, e.g. that parens
//            match and that each statement is ended with a semicolon (if applicable)
//
// --------------------------------------------------------------------------------------------------------------------



// if go_to_next_statement is false, the iterator will be reset to the beginning of the statement
// returns nullptr if no more statements could be read.
std::shared_ptr<Statement> read_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(parent_scope != nullptr && parent_scope->dynamic == false); // This function should only be used during parsing pass 1

    if (it->is_eof()) {
        // no error, just return nullptr as expected
        return nullptr;
    } else if (it->type == Token_type::SYMBOL && it->token == ";") {
        // empty statement, give warning and ignore
        log_warning("Additional ';' found", it->context);
        it.eat_token(); // eat the ';' token
        return read_statement(it, parent_scope);

    } else if (it->type == Token_type::KEYWORD && it->token == "using") {
        // using statement, read and add to special list in scope
        auto us = read_using_statement(it, parent_scope);
        ASSERT(us != nullptr);
        parent_scope->using_statements.push_back(us);
        return std::static_pointer_cast<Statement>(us);

    } else if (it->type == Token_type::SYMBOL && it->token == "{") {
        // nested scope
        auto scope = read_anonymous_static_scope(it, parent_scope);
        return std::static_pointer_cast<Statement>(scope);

    } else if (it->type == Token_type::KEYWORD && it->token == "if") {
        // if statement
        auto if_s = read_if_statement(it, parent_scope);
        return std::static_pointer_cast<Statement>(if_s);

    } else if (it->type == Token_type::KEYWORD && it->token == "for") {
        // for statement
        auto for_s = read_for_statement(it, parent_scope);
        return std::static_pointer_cast<Statement>(for_s);

    } else if (it->type == Token_type::KEYWORD && it->token == "while") {
        // while statement
        auto while_s = read_while_statement(it, parent_scope);
        return std::static_pointer_cast<Statement>(while_s);

    } else if (it->type == Token_type::KEYWORD && it->token == "return") {
        // return statement
        auto ret_s = read_return_statement(it, parent_scope);
        return std::static_pointer_cast<Statement>(ret_s);

    } else if (it->type == Token_type::KEYWORD && it->token == "infix_operator") {
        // infix operator declaration
        auto decl_s = read_operator_declaration(it, parent_scope);
        return std::static_pointer_cast<Statement>(decl_s);

    } else if (it->type == Token_type::KEYWORD && it->token == "prefix_operator") {
        // prefix operator declaration
        auto decl_s = read_operator_declaration(it, parent_scope);
        return std::static_pointer_cast<Statement>(decl_s);

    } else {

        int start_index = it.current_index;
        Token_context context = it->context;

        while (!it.error && !it->is_eof()) {

            // look for ':' (declaration), '=' (assignment) and ';' (unknown statement)
            // the first such matching symbol found determines the type of statement.

            const Token& t = it.eat_token();

            if (t.type == Token_type::SYMBOL) {

                if (t.token == ":") {
                    // declaration statement
                    it.current_index = start_index;
                    auto decl_s = read_declaration_statement(it, parent_scope);
                    return std::static_pointer_cast<Statement>(decl_s);
                }

                if (t.token == "=") {
                    // assignment statement
                    it.current_index = start_index;
                    auto ass_s = read_assignment_statement(it, parent_scope);
                    return std::static_pointer_cast<Statement>(ass_s);
                }

                if (t.token == ";") {
                    // its a line with only a value expression in it
                    // could contain a #run somewhere, but we cant determine that yet
                    it.current_index = start_index;

                    auto us = std::shared_ptr<Unknown_statement>(new Unknown_statement());
                    us->start_token_index = start_index;
                    us->owner = parent_scope;
                    us->context = it.look_at(start_index).context;
                    us->status = Parsing_status::NOT_PARSED;
                    auto global_scope = get_global_scope(parent_scope);
                    global_scope->unknown_statements.push_back(us); // compile it later

                    return std::static_pointer_cast<Statement>(us);
                }

                if (t.token == "(") it.current_index = it.find_matching_paren(it.current_index-1) + 1;  // go back to the previous "(" and search from there
                else if (t.token == "[") it.current_index = it.find_matching_bracket(it.current_index-1) + 1;
                else if (t.token == "{") it.current_index = it.find_matching_brace(it.current_index-1) + 1;

                else if (t.is_eof() || t.token == ")" || t.token == "]" || t.token == "}") {
                    // log_error("Unexpected token "+t.token+" in unresolved statement.", t.context);


                    auto statement = std::shared_ptr<Statement>(new Unknown_statement());
                    statement->start_token_index = start_index;
                    statement->owner = parent_scope;
                    statement->context = it.look_at(start_index).context;
                    statement->status = Parsing_status::FATAL_ERROR;

                    log_error("Missing ';' at the end of statement: expected \";\" before \""+t.token+"\"", t.context);
                    add_note("In unresolved statement that started here: ", statement->context);

                    return statement;
                }
            }
        }
    }

    ASSERT(false); // should never be reached
    return nullptr;
}



Parsing_status fully_resolve_statement(std::shared_ptr<Statement> statement)
{
    if (statement->status == Parsing_status::FULLY_RESOLVED || is_error(statement->status)) {
        // no point in doing anything, just return
        return statement->status;
    }

    // FIXME: Check which type of statement it is, and call the corresponding fully_resolve

    ASSERT(statement->status == Parsing_status::FULLY_RESOLVED || is_error(statement->status));
    return statement->status;
}


std::shared_ptr<Statement> compile_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    log_warning("compile_statement may not work as expected", it->context);
    auto statement = read_statement(it, parent_scope);
    if (!is_error(statement->status)) {
        fully_resolve_statement(statement);
    }
    return statement;
}
