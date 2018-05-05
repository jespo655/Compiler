#include "parser.h"
#include "../abstx/abstx_scope.h"
#include "../abstx/expressions/value_expression.h"
#include "../abstx/expressions/variable_expression.h"
#include "../abstx/expressions/abstx_function.h"
#include "../abstx/expressions/abstx_identifier.h"
#include "../abstx/expressions/abstx_pointer_dereference.h"
#include "../abstx/expressions/abstx_simple_literal.h"

#include "../abstx/statements/abstx_function_call.h"
#include "../abstx/statements/abstx_declaration.h"

#include "../utilities/sequence.h"
#include "../utilities/pointers.h"

#include "../types/all_cb_types.h"
// #include "../compile_time/compile_time.h"




// --------------------------------------------------------------------------------------------------------------------
//
//          Expression examination
//          Used to determine which type of expression to read
//
// --------------------------------------------------------------------------------------------------------------------





// Part 1:
// if token is '(' read value expression with reset operator prio
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
Owned<Value_expression> read_value_expression(Token_iterator& it, Shared<Abstx_scope> parent_scope, int min_operator_prio)
{
    Owned<Value_expression> expr{nullptr};

    ASSERT(!it->is_eof()); // this should have already been checked before

    if (it->type == Token_type::SYMBOL && it->token == "(") {
        // eat the token, then read new value expression, then expect closing paren
        it.eat_token();
        expr = read_value_expression(it, parent_scope);
        it.expect(Token_type::SYMBOL, ")");
        if (it.expect_failed()) {
            add_note("In paren enclosed value expression that started here", expr->context);
        }
        expr->status = Parsing_status::FATAL_ERROR; // mismatched parens -> fatal error

    } else if (it->type == Token_type::SYMBOL && it->token == "[") {
        expr = read_sequence_literal(it, parent_scope);

    } else if (it->type == Token_type::INTEGER || it->type == Token_type::FLOAT || it->type == Token_type::STRING || it->type == Token_type::BOOL) {
        expr = read_simple_literal(it, parent_scope);

    }

    // @TODO: checked to here
    /*
    else if (it->type == Token_type::IDENTIFIER) {
        // This can be either a variable identifier or an infix operator.
        // Create a prefix expr node while it still is untouched, just in case
        auto p_expr = Shared<Prefix_expr>(new Prefix_expr());
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
                auto i_expr = Shared<Infix_expr>(new Infix_expr());
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
    */

    ASSERT(expr != nullptr);
    ASSERT(is_error(expr->status) || expr->status == Parsing_status::FULLY_RESOLVED);
    return std::move(expr);
}









Owned<Value_expression> read_simple_literal(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    Owned<Abstx_simple_literal> o = alloc(Abstx_simple_literal());
    o->set_owner(parent_scope); // temporary owner; the literal should be owned by a statement somewhere
    o->context = it->context;
    o->start_token_index = it.current_index;

    const Token& t = it.eat_token();

    switch(t.type) {
        case Token_type::INTEGER:
            try {
                if (t.token[0] == '-') {
                    // signed integer literal
                    o->value.v_type = CB_Int::type;
                    o->value.v_ptr = alloc_constant_data(CB_Int::type->cb_sizeof());
                    *(CB_Int::c_typedef*)o->value.v_ptr = std::stoll(t.token);
                } else {
                    // unsigned integer literal
                    o->value.v_type = CB_Uint::type;
                    o->value.v_ptr = alloc_constant_data(CB_Uint::type->cb_sizeof());
                    *(CB_Uint::c_typedef*)o->value.v_ptr = std::stoull(t.token);
                }
            } catch (std::out_of_range e) {
                log_error("Integer literal too large to fit!", t.context);
                o->status = Parsing_status::FATAL_ERROR;
            }
            break;
        case Token_type::FLOAT:
            try {
                // float literal
                o->value.v_type = CB_Float::type;
                o->value.v_ptr = alloc_constant_data(CB_Float::type->cb_sizeof());
                *(CB_Float::c_typedef*)o->value.v_ptr = std::stod(t.token);
            } catch (std::out_of_range e) {
                log_error("Float literal too large to fit!", t.context);
                o->status = Parsing_status::FATAL_ERROR;
            }
            break;
        case Token_type::BOOL:
            ASSERT(it->token == "true" || it->token == "false"); // this should be checked by lexer
            // boolean literal
            o->value.v_type = CB_Bool::type;
            o->value.v_ptr = alloc_constant_data(CB_Bool::type->cb_sizeof());
            *(CB_Bool::c_typedef*)o->value.v_ptr = (t.token == "true");
            break;
        case Token_type::STRING:
            o->value.v_type = CB_String::type;
            o->value.v_ptr = alloc_constant_data(CB_String::type->cb_sizeof());
            *(CB_String::c_typedef*)o->value.v_ptr = (CB_String::c_typedef)t.token.c_str(); // valid as long as the list of tokens is alive @warning potentially dangerous
            break;
        default:
            ASSERT(false); // any other type of token cannot be a simple literal
    }
    if (!is_error(o->status)) o->finalize();
    return owned_static_cast<Value_expression>(std::move(o));
}




Owned<Value_expression> read_sequence_literal(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    // @todo
    return nullptr;
}






Owned<Variable_expression> read_function_call(Token_iterator& it, Shared<Abstx_scope> parent_scope, Owned<Variable_expression>&& fn_id, const Seq<Shared<Variable_expression>>& lhs) {
    // syntax: variable_expression()
    // it is inserted as its own statement in the scope
    // if in a declaration statement, the declaration comes first, then the function call,
    //   otherwise the function call comes first.
    // If the function has return values that are immediately used as values for something else,
    //   they have to be declared as temporary identifiers in a separate declaration statement, then the function call is evaluated
    /*
        Example:

        a, b = foo(); // cb
        foo(&a, &b); // c

        a, b := foo(); // cb
        T a; T2 b; // c decl statement
        foo(&a, &b); // c fn call

        a, b := foo(foo(c, d)); // cb
        T a; T2 b; // c decl statement
        T _cb_tmp_1; T2 _cb_tmp_2; // c tmp decl statement
        foo(c, d, &_cb_tmp_1, &_cb_tmp_2); // inner fn call
        foo(_cb_tmp_1, _cb_tmp_2, &a, &b); // outer fn call
    */

    // Allocate abstx node
    Owned<Abstx_function_call> o = alloc(Abstx_function_call());
    o->set_owner(parent_scope); // temporary
    o->context = it->context;
    o->start_token_index = it.current_index;

    // Check function identifier
    Shared<const CB_Type> t = fn_id->get_type();
    ASSERT(t != nullptr); // type must be known
    Shared<const CB_Function> fn_type = dynamic_pointer_cast<const CB_Function>(t);
    if (fn_type == nullptr) {
        log_error("Non-function expression used as a function", o->context);
        o->status = Parsing_status::SYNTAX_ERROR;
    }
    o->function_pointer = std::move(fn_id);

    // Check out_args
    if (lhs.size > 0) o->out_args = lhs;
    else if (fn_type->out_types.size > 0) {
        // create declaration statement with tmp variables; push it to scope
        // add its temporary variables as out_args
        Owned<Abstx_declaration> tmp_decl = alloc(Abstx_declaration());
        tmp_decl->set_owner(parent_scope);
        for (const auto& type : fn_type->out_types) {
            // create tmp identifier; add to declaration statement
            Owned<Abstx_identifier> tmp_id = alloc(Abstx_identifier());
            tmp_id->set_owner(Shared<Abstx_declaration>(tmp_decl));
            tmp_id->context = it->context;
            // tmp_id->start_token_index = ---; // no start token index since the token doesn't really exist
            tmp_id->name = "_cb_tmp_" + std::to_string(get_unique_id());
            tmp_id->value.v_type = type;

            o->out_args.add(static_pointer_cast<Variable_expression>(tmp_id));
            tmp_decl->identifiers.add(std::move(tmp_id));
        }
        parent_scope->statements.add(owned_static_cast<Statement>(std::move(tmp_decl)));
    }

    it.assert(Token_type::SYMBOL, "("); // this should already have been checked

    while (1) {
        if (it.compare(Token_type::SYMBOL, ")")) break; // done

        Owned<Value_expression> arg = read_value_expression(it, parent_scope);
        arg->set_owner(o);
        ASSERT(arg != nullptr);
        if (is_error(arg->status)) {
            o->status = arg->status;
            if (is_fatal(arg->status)) {
                break;
            }
        }

        // if the value expression is a function call, add its out arguments as in arguments to this function call
        Shared<Abstx_function_call_expression> fc = dynamic_pointer_cast<Abstx_function_call_expression>(arg);
        if (fc != nullptr) {
            ASSERT(fc->function_call != nullptr);
            if (fc->function_call->out_args.size == 0) {
                for (const Shared<Variable_expression>& out_arg : fc->function_call->out_args) {
                    Owned<Variable_expression_reference> id_ref = alloc(Variable_expression_reference());
                    id_ref->set_owner(o);
                    id_ref->context = fc->context;
                    id_ref->start_token_index = fc->start_token_index;
                    id_ref->expr = out_arg;
                    o->in_args.add(owned_static_cast<Value_expression>(std::move(id_ref)));
                }
            }
        } else {
            // not a function call -> just add the argument directly
            o->in_args.add(std::move(arg));
        }


        if (it.compare(Token_type::SYMBOL, ",")) {
            it.eat_token();
            continue;
        } else if (!it.compare(Token_type::SYMBOL, ")")) {
            log_error("Missing ')' at end of function call", it->context);
            o->status = Parsing_status::FATAL_ERROR;
            break;
        }
    }

    if (!is_error(o->status)) {
        it.assert(Token_type::SYMBOL, ")"); // this is already checked / now eat the token
    } else if (!is_fatal(o->status)) {
        it.current_index = it.find_matching_paren() + 1;
        if (it.expect_failed()) {
            o->status = Parsing_status::FATAL_ERROR;
        }
    }
    Owned<Abstx_function_call_expression> expr;
    expr->set_owner(parent_scope);
    expr->context = o->context;
    expr->start_token_index = o->start_token_index;
    expr->function_call = o;

    parent_scope->statements.add(owned_static_cast<Statement>(std::move(o)));

    // Abstx_function_call_expression
    return owned_static_cast<Variable_expression>(std::move(expr));
}







/*
const int eval_prio = 1000; // FIXME: fix. Maybe use the current operator prio?


Shared<Literal> compile_eval_expr(Token_iterator& it, Shared<Abstx_scope> parent_scope)
{
    auto lit = Shared<Literal>(new Literal());
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








Shared<Value_expression> compile_value_list(Token_iterator& it, Shared<Abstx_scope> parent_scope)
{
    auto v_list = Shared<Value_list>(new Value_list());
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










Shared<Variable_expression> compile_variable_expression(Token_iterator& it, Shared<Abstx_scope> parent_scope)
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

*/



















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



