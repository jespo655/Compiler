#include "parser.h"
#include "../abstx/defer.h"



// for statement during static scope: log error and go to the end of statement.
std::shared_ptr<Defer_statement> read_defer_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT (parent_scope != nullptr && !parent_scope->dynamic); // during pass 2, use compile_defer_statement instead.

    auto ds = std::shared_ptr<Defer_statement>(new Defer_statement());

    ds->owner = parent_scope;
    ds->context = it->context;
    ds->start_token_index = it.current_index;
    ds->status = Parsing_status::SYNTAX_ERROR;

    log_error("Defer statement not allowed in static scope", it->context);

    // syntax:
    // defer statement;

    it.assert(Token_type::KEYWORD, "defer");
    it.current_index = it.find_matching_semicolon() + 1;

    if (it.error) ds->status = Parsing_status::FATAL_ERROR;
    return ds;

}



std::shared_ptr<Defer_statement> compile_defer_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT (parent_scope != nullptr && parent_scope->dynamic); // during pass 1, use read_defer_statement instead.

    auto ds = std::shared_ptr<Defer_statement>(new Defer_statement());

    ds->owner = parent_scope;
    ds->context = it->context;
    ds->start_token_index = it.current_index;

    // syntax:
    // defer statement;

    it.assert(Token_type::KEYWORD, "defer");
    ds->statement = compile_statement(it, parent_scope);
    ASSERT(ds->statement != nullptr);
    ds->staus = ds->statement->status;

    return ds;
}
