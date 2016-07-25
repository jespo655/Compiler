#include "parser.h"
#include "../abstx/for.h"



// for statement during static scope: log error and go to the end of statement.
std::shared_ptr<For_statement> read_for_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT (parent_scope != nullptr && !parent_scope->dynamic); // during pass 2, use compile_if_statement instead.

    auto fs = std::shared_ptr<For_statement>(new For_statement());

    fs->owner = parent_scope;
    fs->context = it->context;
    fs->start_token_index = it.current_index;
    fs->status = Parsing_status::SYNTAX_ERROR;

    log_error("For statement not allowed in static scope", it->context);

    // syntax:
    // for () {}

    it.assert(Token_type::KEYWORD, "for");
    it.expect_current(Token_type::SYMBOL, "(");
    if (!it.error) it.current_index = it.find_matching_paren() + 1;
    if (!it.error) it.expect_current(Token_type::SYMBOL, "{");
    if (!it.error) it.current_index = it.find_matching_brace() + 1;

    if (it.error) fs->status = Parsing_status::FATAL_ERROR;
    return fs;

}



std::shared_ptr<For_statement> compile_for_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(false, "NYI");
}
