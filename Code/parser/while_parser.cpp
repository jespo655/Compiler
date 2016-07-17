#include "parser.h"
#include "../abstx/while.h"



// while statement during static scope: log error and go to the end of statement.
std::shared_ptr<While_statement> read_while_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT (parent_scope != nullptr && !parent_scope->dynamic); // during pass 2, use compile_if_statement instead.

    auto ws = std::shared_ptr<While_statement>(new While_statement());

    ws->owner = parent_scope;
    ws->context = it->context;
    ws->start_token_index = it.current_index;
    ws->status = Parsing_status::SYNTAX_ERROR;

    log_error("While statement not allowed in static scope", it->context);

    // syntax:
    // while () {}

    it.assert(Token_type::KEYWORD, "while");
    it.expect_current(Token_type::SYMBOL, "(");
    if (!it.error) it.current_index = it.find_matching_paren() + 1;
    if (!it.error) it.expect_current(Token_type::SYMBOL, "{");
    if (!it.error) it.current_index = it.find_matching_brace() + 1;

    if (it.error) ws->status = Parsing_status::FATAL_ERROR;
    return ws;

}



std::shared_ptr<While_statement> compile_while_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(false, "NYI");
}