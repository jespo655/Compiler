#include "parser.h"
#include "../abstx/value_list.h"
#include "../abstx/scope.h"
#include "../abstx/Seq.h"
#include "../abstx/operators.h"
#include "../compile_time/compile_time.h"

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
        lit->value = eval(expr);
        ASSERT(lit->value.is_allocated());
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
        v_list->status = Parsing_status::FULLY_RESOLVED;
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
// else if token is '[' read Seq literal
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

    ASSERT(!it->is_eof());

    if (it->type == Token_type::COMPILER_COMMAND && it->token == "#eval") {
        auto lit = compile_eval_expr(it, parent_scope);
        expr = std::static_pointer_cast<Value_expression>(lit);

    } else if (it->type == Token_type::SYMBOL && it->token == "(") {
        auto vl = compile_value_list(it, parent_scope);
        expr = std::static_pointer_cast<Value_expression>(vl);

    } else if (it->type == Token_type::SYMBOL && it->token == "[") {
        expr = compile_sequence_literal(it, parent_scope);

    } else if (it->type == Token_type::INTEGER) {
        expr = compile_int_literal(it, parent_scope);

    } else if (it->type == Token_type::FLOAT) {
        expr = compile_float_literal(it, parent_scope);

    } else if (it->type == Token_type::STRING) {
        expr = compile_string_literal(it, parent_scope);

    } else if (it->type == Token_type::BOOL) {
        expr = compile_bool_literal(it, parent_scope);

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
            expr = std::static_pointer_cast<Value_expression>(p_expr);
        } else {
            expr = std::static_pointer_cast<Value_expression>(identifier);
        }

    } else {
        log_error("Unable to parse expression",it->context);
        ASSERT(false, "FIXME: what to do?");
    }

    if (expr->status == Parsing_status::FATAL_ERROR) return expr;
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
            if (op_prio > min_operator_prio || right_associative && op_prio == min_operator_prio) {
                auto i_expr = std::shared_ptr<Infix_expr>(new Infix_expr());
                i_expr->owner = parent_scope;
                i_expr->start_token_index = it.current_index;
                i_expr->context = it->context;
                it.eat_token();

                i_expr->lhs = expr;
                i_expr->rhs = compile_value_expression(it, parent_scope, op_prio);
                // FIXME: check for fatal error
                i_expr->rhs->owner = i_expr;
                expr->owner = i_expr;
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





















// COMMENTS FROM OLD .h-FILE



/*
Parsing expressions are the most complex part of the parsing process.
There are so many special cases and complex complications that this deserves to be in a separate file.
*/



// These parsing functions start at the current iterator position, and reads one expression.
// The parsing will stop at the first encountered ',', ')', or any other unexpected token (without logging an error).
// When the function returns, the iterator will point to said unexpected token.



/*
P   prefix operator
S   suffix operator
I   infix operator

High priority special operators:
()  function call (as suffix), or a sequence of expressions (surrounding other expressions)
[]  array indexing (as suffix), or an array literal (surrounding other expressions)
 MAYBE: || absolute value (surrounding other expressions that evaluate to numbmers)
.   getter



Valid expressions: (where expr is an expression and expr_list is a comma separated list of expressions)

literal             (bool, int, float, string)
identifier
P expr
expr S
expr I expr
expr(expr_list)
(expr_list)
expr[expr]
expr . identifier


A value expression is a variable expression if:
    * Getter
    * Array indexing
    * Identifier
    * Function or operator that returns a pointer


Complex expressions

P P P expr              // P(P(P(expr)))
expr S S S              // (((expr)S)S)S
expr I1 expr I2 expr    // (expr I1 expr) I2 expr if I1.prio>=I2.prio, else expr I1 (expr I2 expr)
P expr I expr           // (P expr) I1 expr if P.prio>=I.prio, else P(expr I expr)
expr I expr S           // (expr I1 expr) S if I.prio>=S.prio, else expr I (expr S)
P expr S                // (P expr) S if P.prio>=I.prio, else P(expr S)
expr S I expr           // (expr S) I1 expr
expr I P expr           // expr I1 (P expr)

Even more complex: if @ is both P, I and S at the same time:

expr @ expr             // @ is I
@ expr                  // @ is P
expr @                  // @ is S
expr @1 @2 expr         // @1=I, @2=P OR @1=S, @2=I. If both these combinations are possible, then logs an error





Error handling:

expr P I expr           // syntax error, "P can't modify an operator"
expr SI PI expr         // SI is suffix and infix, and PI is infix and prefix. syntax error "Unable to resolve order of operators"









inbyggda operatorer:

infix_operator .. := fn(low:$N1, high:$N2)->r:range
{
    #run assert_number(N1);
    #run assert_number(N2);
    r.low = low.float; // guaranteed to work after the assert
    r.high = high.float;
};



infix



*/




// Backup-functions if genereics doesn't work out:

// infix_operator .. := fn(low:float, high:float)->r:range { r.low = low; r.high = high; };
// infix_operator .. := fn(low:float, high:int)->r:range { r.low = low; r.high = high.float; };
// infix_operator .. := fn(low:int, high:int)->r:range { r.low = low.float; r.high = high.float; };



