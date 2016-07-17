#include "parser.h"
#include "../abstx/statement.h"



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
        return us;
        // return std::static_pointer_cast<Statement>(us);

    } else if (it->type == Token_type::SYMBOL && it->token == "{") {
        // nested scope
        if (parent_scope->dynamic) return read_dynamic_scope(it, parent_scope);
        else return read_static_scope(it, parent_scope);
        // return std::static_pointer_cast<Statement>(scope);

    } else if (it->type != Token_type::STRING && it->token == "if") {
        // if statement
        return read_if_statement(it, parent_scope);
        // return std::static_pointer_cast<Statement>(if_s);
    }

    else if (it->type != Token_type::STRING && it->token == "for") {
        // for statement
        return read_for_statement(it, parent_scope);
        // return std::static_pointer_cast<Statement>(for_s);

        // FIXME: move this error message to read_for_statement()
        // if (!parent_scope->dynamic) log_error("for statement not allowed in static scope", it->context); // FIXME: proper error message
    }

    else if (it->type != Token_type::STRING && it->token == "while") {
        // while statement
        return read_while_statement(it, parent_scope);
        // return std::static_pointer_cast<Statement>(while_s);

        // FIXME: move this error message to read_while_statement()
        // if (!parent_scope->dynamic) log_error("while statement not allowed in static scope", it->context); // FIXME: proper error message
    }

    else if (it->type != Token_type::STRING && it->token == "return") {
        // return statement
        return read_return_statement(it, parent_scope);

        // FIXME: move this error message to read_return_statement()
        // if (!parent_scope->dynamic) log_error("Return statement not allowed in static scope", it->context); // FIXME: proper error message
    }

    else {

        int start_index = it.current_index;
        Token_context context = it->context;

        while (!it.error && !it->is_eof()) {

            // look for ':' (declaration), '=' (assignment) and ';' (unknown statement)
            // the first such matching symbol found determines the type of statement.

            // If an unknown statement ends with ");", then it might be a function call, but it's not certain.

            const Token& t = it.eat_token();

            if (t.type == Token_type::SYMBOL) {

                if (t.token == ":") {
                    // declaration statement
                    it.current_index = start_index;
                    return read_declaration_statement(it, parent_scope);
                }

                if (t.token == "=") {
                    // assignment statement
                    it.current_index = start_index;
                    return read_assignment_statement(it, parent_scope);
                }

                if (t.token == ";") {
                    // its a line with only a value expression in it
                    it.current_index = start_index;
                    return read_expression_statement(it, parent_scope);
                }

                if (t.token == "(") it.current_index = it.find_matching_paren(it.current_index-1) + 1;  // go back to the previous "(" and search from there
                else if (t.token == "[") it.current_index = it.find_matching_bracket(it.current_index-1) + 1;
                else if (t.token == "{") it.current_index = it.find_matching_brace(it.current_index-1) + 1;

                else if (t.token == ")" || t.token == "]" || t.token == "}") {
                    log_error("Unexpected token "+t.token+" in unresolved statement.", t.context);
                    auto statement = std::shared_ptr<Statement>(new Unknown_statement());
                    statement->start_token_index = start_index;
                    statement->owner = parent_scope;
                    statement->context = context;
                    statement->status = Parsing_status::FATAL_ERROR;
                    it.error = true;
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
