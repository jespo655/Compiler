#include "parser.h"
#include "../abstx/if.h"


// if statement during static scope: log error and go to the end of statement.
std::shared_ptr<If_statement> read_if_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT (parent_scope != nullptr && !parent_scope->dynamic); // during pass 2, use compile_if_statement instead.

    auto is = std::shared_ptr<If_statement>(new If_statement());

    is->owner = parent_scope;
    is->context = it->context;
    is->start_token_index = it.current_index;
    is->status = Parsing_status::SYNTAX_ERROR;

    log_error("If statement not allowed in static scope", it->context);

    // syntax:
    // if () {} elsif () {} elsif () {} else {} then {}

    it.assert_current(Token_type::KEYWORD, "if");
    do {
        it.eat_token(); // eat the "if" or "elsif" token
        it.expect_current(Token_type::SYMBOL, "(");
        if (!it.error) it.current_index = it.find_matching_paren() + 1;
        if (!it.error) it.expect_current(Token_type::SYMBOL, "{");
        if (!it.error) it.current_index = it.find_matching_brace() + 1;
    } while (!it.error && it->type == Token_type::KEYWORD && it->token == "elsif");

    if (!it.error && it->type == Token_type::KEYWORD && it->token == "else") {
        it.eat_token(); // eat the "if" or "elsif" token
        it.expect_current(Token_type::SYMBOL, "{");
        if (!it.error) it.current_index = it.find_matching_brace() + 1;
    }

    if (!it.error && it->type == Token_type::KEYWORD && it->token == "then") {
        it.eat_token(); // eat the "if" or "elsif" token
        it.expect_current(Token_type::SYMBOL, "{");
        if (!it.error) it.current_index = it.find_matching_brace() + 1;
    }

    if (it.error) is->status = Parsing_status::FATAL_ERROR;
    return is;

}









std::shared_ptr<If_statement> compile_if_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{

    ASSERT (parent_scope != nullptr && parent_scope->dynamic); // during pass 2, use compile_if_statement instead.

    auto is = std::shared_ptr<If_statement>(new If_statement());

    is->owner = parent_scope;
    is->context = it->context;
    is->start_token_index = it.current_index;

    // syntax:
    // if (bool_expr) {} elsif (bool_expr) {} elsif (bool_expr) {} else {} then {}

    it.assert_current(Token_type::KEYWORD, "if");
    do {
        auto cs = std::shared_ptr<Conditional_scope>(new Conditional_scope());
        cs->owner = is;
        cs->start_token_index = it.current_index;
        cs->context = it->context;

        cs->scope = std::shared_ptr<Scope>(new Scope());
        cs->scope->owner = cs;
        // set start token and context a bit later

        it.eat_token(); // eat the "if" or "elsif" token
        it.expect_current(Token_type::SYMBOL, "(");
        if (it.error) {
            is->status = Parsing_status::FATAL_ERROR;
            return is;
        }

        cs->condition = compile_value_expression(it, parent_scope); // maybe parent scope is cs->scope?
        ASSERT(cs->condition != nullptr);
        cs->condition->owner = cs;

        if (cs->condition->status == FATAL_ERROR) {
            is->status = Parsing_status::FATAL_ERROR;
            return is;
        }

        if (auto bool_t = std::dynamic_pointer_cast<Type_bool>(cs->condition->get_type())) {
            // all is ok
        } else if (!is_error(cs->condition->status)) {
            log_error("Type mismatch in if condition: expected bool type but found "+cs->condition->get_type()->toS(), cs->condition->context);
            cs->status == Parsing_status::TYPE_ERROR;
        } else {
            cs->status == cs->condition->status;
        }

        if (!it.error) it.expect_current(Token_type::SYMBOL, "{");
        if (!it.error) {
            cs->scope = compile_dynamic_scope(it, parent_scope);
            ASSERT(cs->scope != nullptr);
            cs->scope->owner = cs;
            if (is_error(cs->scope->status)) cs->status = cs->scope->status;
        }

        if (it.error || cs->status == Parsing_status::FATAL_ERROR) {
            is->status = Parsing_status::FATAL_ERROR;
            return is;
        } else if (is_error(cs->status)) {
            is->status = cs->status;
            is->conditional_scopes.push_back(cs);
        }

    } while (it->type == Token_type::KEYWORD && it->token == "elsif");

    if (!it.error && it->type == Token_type::KEYWORD && it->token == "else") {
        it.eat_token(); // eat the "if" or "elsif" token
        it.expect_current(Token_type::SYMBOL, "{");
        if (!it.error) {
            is->else_scope = compile_dynamic_scope(it, parent_scope);
            is->else_scope->owner = is;
            if (is_error(is->else_scope)) is->status = is->else_scope->status;
        }
    }

    if (!it.error && it->type == Token_type::KEYWORD && it->token == "then") {
        it.eat_token(); // eat the "if" or "elsif" token
        it.expect_current(Token_type::SYMBOL, "{");
        if (!it.error) {
            is->then_scope = compile_dynamic_scope(it, parent_scope);
            is->then_scope->owner = is;
            if (is_error(is->then_scope)) is->status = is->then_scope->status;
        }
    }

    if (it.error) is->status = Parsing_status::FATAL_ERROR;
    return is;
}