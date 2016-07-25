#include "parser.h"
#include "../abstx/assignment.h"




std::shared_ptr<Assignment_statement> read_assignment_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT (parent_scope != nullptr && !parent_scope->dynamic); // during pass 2, use compile_if_statement instead.

    auto as = std::shared_ptr<Assignment_statement>(new Assignment_statement());

    as->owner = parent_scope;
    as->context = it->context;
    as->start_token_index = it.current_index;
    as->status = Parsing_status::SYNTAX_ERROR;

    log_error("Assignment statement not allowed in static scope", it->context);

    // syntax:
    // var_expr, var_expr, var_expr = val_expr, val_expr, val_expr;

    it.current_index = it.find_matching_semicolon() + 1;

    if (it.error) as->status = Parsing_status::FATAL_ERROR;
    return as;
}




std::shared_ptr<Assignment_statement> compile_assignment_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(false, "NYI");
}
