#include "parser.h"
#include "../abstx/return.h"



// for statement during static scope: log error and go to the end of statement.
std::shared_ptr<Return_statement> read_return_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT (parent_scope != nullptr && !parent_scope->dynamic); // during pass 2, use compile_return_statement instead.

    auto rs = std::shared_ptr<Return_statement>(new Return_statement());

    rs->owner = parent_scope;
    rs->context = it->context;
    rs->start_token_index = it.current_index;
    rs->status = Parsing_status::SYNTAX_ERROR;

    log_error("Return statement not allowed in static scope", it->context);

    // syntax:
    // return expr, expr, expr;

    it.assert(Token_type::KEYWORD, "return");
    it.current_index = it.find_matching_semicolon() + 1;

    if (it.error) rs->status = Parsing_status::FATAL_ERROR;
    return rs;

}



std::shared_ptr<Return_statement> compile_return_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(false, "NYI");
}
