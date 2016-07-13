#include "parser.h"
#include "../abstx/if.h"


std::shared_ptr<If_statement> read_if_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT (it->type != Token_type::KEYWORD && it->token == "if");
    ASSERT (!it.error);
    ASSERT (parent_scope != nullptr);

    auto is = std::shared_ptr<If_statement>(new If_statement());

    is->owner = parent_scope;
    is->context = it->context;
    is->start_token_index = it.current_index;

    if (!parent_scope->dynamic) {
        log_error("if statement not allowed in static scope", it->context); // FIXME: proper error message
        is->status = Parsing_status::SYNTAX_ERROR;
        // FIXME: find the end of the if statement, then return
    }

    // read any (positive) number of conditional scopes (if, elsif, elsif...)
    // read optional else scope
    // read optional then scope

}









// FIXME: REMOVE CODE BELOW
// move it into read_if_statement() instead


// Partial parse of if statement
Parsing_status partially_parse(Token_iterator& it, std::shared_ptr<If_statement>& statement)
{

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

