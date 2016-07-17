#include "expression_parser.h"
#include "../abstx/value_list.h"
#include "../abstx/scope.h"
#include "../abstx/seq.h"

const int eval_prio = 1000; // FIXME: fix. Maybe use the current operator prio?





std::shared_ptr<Literal> compile_eval_expr(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    auto lit = std::shared_ptr<Literal>(new Literal());
    lit->owner = parent_scope;
    lit->start_token_index = it.current_index;
    lit->context = it->context;

    it.assert(Token_type::COMPILER_COMMAND, "#eval"); // eat the eval token
    auto expr = compile_value_expression(it, parent_scope, eval_prio);

    lit->status = expr->status;

    if (expr->status == Parsing_status::FULLY_RESOLVED) {
        lit.value = eval(expr);
        ASSERT(lit.value.is_allocated());
    } else {
        log_error("Unable to evaluate expression",lit->context);
    }

    return lit;
}








std::shared_ptr<Value_expression> compile_value_list(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    auto v_list = std::shared_ptr<Value_list>(new Value_list());
    v_list->owner = parent_scope;
    v_list->context = it->context;
    v_list->start_token_index = it.current_index;

    it.assert(Token_type::SYMBOL, "("); // eat the "(" token

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
            if (it.error) v_list->status = Parsing_status::FATAL_ERROR;
            return v_list;
        }

        auto value_expr = compile_value_expression(it, parent_scope);
        ASSERT(value_expr != nullptr);
        value_expr->owner = v_list;
        v_list->expressions.push_back(value_expr);
        if (is_error(value_expr->status)) {
            v_list->status = value_expr->status;
        } else ASSERT(value_expr->status == Parsing_status::FULLY_RESOLVED);

        first = false;
    }

    if (!is_error(v_list->status)) {
        v_list->status = Parsing_status::FULLY_RESOLVED);
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
std::shared_ptr<Value_expression> compile_value_expression(Token_iterator& it, std::shared_ptr<Scope> parent_scope, int min_operator_prio)
{
    std::shared_ptr<Value_expression> expr{nullptr};

    ASSERT(!it->eof());

    if (it->type == Token_type::COMPILER_COMMAND && it->token == "#eval") {
        auto lit = compile_eval_expr(it, parent_scope);
        expr = std::static_pointer_cast<Value_expression>(lit);

    } else if (it->type == Token_type::SYMBOL && it->token == "(") {
        auto vl = compile_value_list(it, parent_scope);
        expr = std::static_pointer_cast<Value_expression>(vl);

    } else if (it->type == Token_type::SYMBOL && it->token == "[") {
        expr = compile_sequence_literal(it, parent_scope);

    } else if (it->type == Token_type::INTEGER) {
        expr = read_integer_literal(it, parent_scope);

    } else if (it->type == Token_type::FLOAT) {
        expr = read_float_literal(it, parent_scope);

    } else if (it->type == Token_type::STRING) {
        expr = read_string_literal(it, parent_scope);

    } else if (it->type == Token_type::BOOL) {
        expr = read_bool_literal(it, parent_scope);

    } else if (it->type == Token_type::IDENTIFIER) {
        // This can be either a variable identifier or an infix operator.
        // Create a prefix expr node while it still is untouched, just in case
        auto p_expr = std::shared_ptr<Prefix_expr>(new Prefix_expr());
        p_expr->owner = parent_scope;
        p_expr->start_token_index = it.current_index;
        p_expr->context = it->context;

        auto identifier = compile_identifier(it, parent_scope);
        // FIXME: handle undeclared identifier

        if (auto op_t = std::dynamic_pointer_cast<Type_operator>(identifier->get_type())) {
            // FIXME: rewrite the operator system (abstx)
            p_expr->op_id = identifier;
            p_expr->expr = compile_value_expression(it, parent_scope, op_t->get_prio());
            p_expr->expr->owner = p_expr;
            expr = static_pointer_cast<Value_expression>(p_expr);
        } else {
            expr = static_pointer_cast<Value_expression>(identifier);
        }

    } else {
        log_error("Unable to parse expression",it->context);
        ASSERT(false "FIXME: what to do?");
    }

    if (expr->status == FATAL_ERROR) return expr;
    ASSERT(is_error(expr->status) || expr->status == Parsing_status::FULLY_RESOLVED);

    // Part 2: chained suffix operators
    // expr = compile_chained_suffixes(it, expr);
    while (true) {
        if (it->type == Token_type::SYMBOL && it->token == "(") {
            expr = compile_function_call(it, expr);
        } else if (it->type == Token_type::SYMBOL && it->token == "[") {
            expr = compile_seq_indexing(it, expr);
        } else if (it->type == Token_type::SYMBOL && it->token == ".") {
            expr = compile_getter(it, expr);
        } else if (it->type == Token_type::SYMBOL || it->type == Token_type::IDENTIFIER) {
            ASSERT(false, "infix operators NYI");
            // could be an infix opreator. Check if it is.
            // If it isn't, break. Otherwise:

            int op_prio = 0; // FIXME: set correctly
            bool right_associative = false; // FIXME: set correctly
            if (op_prio > min_op_prio || right_associative && op_prio == min_op_prio) {
                auto i_expr = std::shared_ptr<Prefix_expr>(new Prefix_expr());
                i_expr->owner = parent_scope;
                i_expr->start_token_index = it.current_index;
                i_expr->context = it->context;
                it.eat_token();

                i_expr->lhs = expr;
                i_expr->rhs = rhs;
                expr->owner = i_expr;
                rhs->owner = i_expr;
                expr = std::static_pointer_cast<Value_expression>(i_expr);
            } else {
                break;
            }

        } else {
            break;
        }
    }

    ASSERT(expr != nullptr);
    ASSERT(is_error(expr->status) || expr->status == Parsing_status::FULLY_RESOLVED);
    return expr;
}


std::shared_ptr<Variable_expression> compile_variable_expression(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    auto context = it->context;
    auto value_expr = compile_value_expression(it, parent_scope);
    ASSERT(value_expr != nullptr);
    if (auto var_expr = std::dynamic_pointer_cast<Variable_expression>(value_expr)) {
        return var_expr; // ok
    }

    log_error("Found value expression in a context where a variable expression was expected", context);
    return nullptr;
}







