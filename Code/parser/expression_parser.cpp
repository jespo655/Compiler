#include "expression_parser.h"
#include "../abstx/value_list.h"
#include "../abstx/scope.h"
#include "../abstx/seq.h"






std::shared_ptr<Literal> compile_eval_expr(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    auto lit = std::shared_ptr<Literal>(new Literal());
    lit->owner = parent_scope;
    lit->start_token_index = it.current_index;
    lit->context = it->context;

    it.assert(Token_type::COMPILER_COMMAND, "#eval"); // eat the eval token
    auto expr = compile_value_expression(it, parent_scope);

    lit->status = expr->status;

    if (expr->status == Parsing_status::FULLY_RESOLVED) {
        lit.value = eval(expr);
        ASSERT(lit.value.is_allocated());
    } else {
        log_error("Unable to evaluate expression",lit->context);
    }

    return lit;
}




















std::shared_ptr<Value_expression> parse_value_list(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<Value_expression> parse_sequence_literal(Token_iterator& it, std::shared_ptr<Scope> parent_scope);




// parse_value_list: 43 lines
std::shared_ptr<Value_expression> parse_value_list(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(it->type == Token_type::SYMBOL && it->token == "(");
    ASSERT(!it.error);

    auto v_list = std::shared_ptr<Value_list>(new Value_list());
    v_list->owner = parent_scope;
    v_list->context = it->context;
    v_list->start_token_index = it.current_index;

    it.eat_token(); // eat the "(" token

    bool first = true;
    while(true) {
        if (it->type == Token_type::SYMBOL && it->token == ")") {
            it.eat_token();
            break; // ok
        }
        if (!first) it.expect(Token_type::SYMBOL, ",");
        if (it.error) {
            v_list->status = Parsing_status::SYNTAX_ERROR;
            it.current_index = it.find_matching_token(it.current_index, Token_type::SYMBOL, ")", "Value_list", "Missing \")\"") + 1;
            ASSERT(!it.error); // we know that the ')' exists because the statement passed partial scope parsing
            break;
        }

        auto value_expr = parse_value_expression(it, parent_scope);
        ASSERT(value_expr != nullptr);
        value_expr->owner = v_list;
        v_list->expressions.push_back(value_expr);
        if (is_error(value_expr->status)) {
            v_list->status = value_expr->status;
        } else ASSERT(value_expr->status == Parsing_status::FULLY_RESOLVED);

        first = false;
    }

    if (!is_error(v_list->status)) {
        v_list->status = Parsing_status::FULLY_RESOLVED)
}

return v_list;
}









// reserved symbols:

// ()
// []
// {}
// :

// Kanske:
// .
// ->
// =











std::shared_ptr<Value_expression> parse_value_expression(Token_iterator& it, std::shared_ptr<Scope> parent_scope, int min_operator_prio)
{
    // Part 1:
    // if token is '(' read value list
    // else if token is '[' read seq literal
    // else if token is integer, float, bool or string literal, construct appropriate literal node
    // else if token is symbol or identifier, check if it's a prefix operator
    //      if it is, read value expression (with min_prio = op.prio), then construct prefix operator node
    // else if token is identifier, check if the identifier exists. If not, log error undeclared variable
    // if none of those applies, log error unexpected token

    // Part 2: suffix operators:
    // '(' function operator
    // '[' indexing operator
    // '.' getter operator
    // other identifier or symbol token: check if it's an infix operator with prio > min_prio
    //      if it is, read value expression (with min_prio = op.prio), then construct infix operator node
    return nullptr;
}


std::shared_ptr<Variable_expression> parse_variable_expression(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    auto context = it->context;
    auto value_expr = parse_value_expression(it, parent_scope);
    if (auto var_expr = std::dynamic_pointer_cast<Variable_expression>(value_expr)) {
        return var_expr; // ok
    }

    if (value_expr != nullptr)
        log_error("Found value expression in a context where a variable expression was expected",context);
    else
        log_error("Unable to parse variable expression",context);

    return nullptr;
}



































































/*

Parsing_status parse_comma_separated_values(
    std::shared_ptr<Scope> parent_scope,
    std::shared_ptr<Abstx_node> owner,
    std::vector<std::shared_ptr<Value_expression>>& expressions,
    Token_type  expected_closing_type,
    std::string expected_closing_token);
{
    Parsing_status status = Parsing_status::NOT_PARSED;
    bool first = true;
    while(true) {
        if (it->type == expected_closing_type && it->token == expected_closing_token) {
            it.eat_token();
            break; // ok
        }
        if (!first) it.expect(Token_type::SYMBOL, ",");
        if (it.error) {
            status = Parsing_status::SYNTAX_ERROR;
            it.current_index = find_matching_token(it.current_index, expected_closing_token, expected_closing_token, "Missing \""+expected_closing_token+"\"") + 1;
            ASSERT(!it.error); // we know that the token exists because the statement passed partial scope parsing
            break;
        }

        auto value_expr = parse_value_expression(it, parent_scope); // FIXME: add parent_scope to fn signature
        ASSERT(value_expr != nullptr);
        value_expr->owner = owner;
        expressions.push_back(value_expr);
        if (is_error(value_expr->status)) {
            status = value_expr->status;
        } else ASSERT(value_expr->status == Parsing_status::FULLY_RESOLVED);

        first = false;
    }
}
*/
