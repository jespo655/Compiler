#include "parser.h"
#include "../abstx/abstx_scope.h"
#include "../abstx/expressions/value_expression.h"
#include "../abstx/expressions/variable_expression.h"
#include "../abstx/expressions/abstx_function.h"
#include "../abstx/expressions/abstx_identifier.h"
#include "../abstx/expressions/abstx_identifier_reference.h"
#include "../abstx/expressions/abstx_pointer_dereference.h"
#include "../abstx/expressions/abstx_struct_getter.h"
#include "../abstx/expressions/abstx_simple_literal.h"
#include "../abstx/expressions/abstx_struct_literal.h"

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

// If unable to read expression, nullpointer is returned
Owned<Value_expression> read_value_expression(Token_iterator& it, Shared<Abstx_node> owner, int min_operator_prio)
{
    Owned<Value_expression> expr = nullptr;

    ASSERT(!it->is_eof()); // this should have already been checked before

    if (it->type == Token_type::SYMBOL && it->token == "(") {
        // eat the token, then read new value expression, then expect closing paren
        it.eat_token();
        expr = read_value_expression(it, owner);
        it.expect(Token_type::SYMBOL, ")");
        if (it.expect_failed()) {
            add_note("In paren enclosed value expression that started here", expr->context);
        }
        expr->status = Parsing_status::FATAL_ERROR; // mismatched parens -> fatal error

    } else if (it->type == Token_type::SYMBOL && it->token == "[") {
        expr = read_sequence_literal(it, owner);

    } else if (it->type == Token_type::KEYWORD && it->token == "fn") {
        expr = read_fn_literal(it, owner);

    } else if (it->type == Token_type::KEYWORD && it->token == "struct") {
        expr = read_struct_literal(it, owner);

    } else if (it->type == Token_type::INTEGER || it->type == Token_type::FLOAT || it->type == Token_type::STRING || it->type == Token_type::BOOL) {
        expr = read_simple_literal(it, owner);

    } else if (it->type == Token_type::IDENTIFIER) {
        expr = read_identifier_reference(it, owner);

        // This can be either a variable identifier or an infix operator.

        // @TODO: (operators) find the type of the identifier
        //      if prefix operator type, check if there are a next identifier to read
        //      if so, create a function call expression instead

    } else {
        log_error("Unable to parse expression",it->context);
        return nullptr;
    }

    if (expr == nullptr) return expr;
    if (expr->status == Parsing_status::FATAL_ERROR) return expr;

    // all expressions should be able to be fully resolved immediately
    if (!(is_error(expr->status) || expr->status == Parsing_status::FULLY_RESOLVED || expr->status == Parsing_status::DEPENDENCIES_NEEDED)) {
        std::cerr << "unexpected expr status: " << expr->status << std::endl; // @debug
    }
    ASSERT(is_error(expr->status) || expr->status == Parsing_status::FULLY_RESOLVED || expr->status == Parsing_status::DEPENDENCIES_NEEDED);

    // Part 2: chained suffix operators
    // expr = compile_chained_suffixes(it, expr);
    while (true) {
        if (it->type == Token_type::SYMBOL && it->token == "(") {
            // @TODO: find a way to pass LHS all the way from declaration statement to here

            Owned<Variable_expression> variable_expr = owned_dynamic_cast<Variable_expression>(std::move(expr));

            if (variable_expr == nullptr) {
                ASSERT(expr != nullptr);
                log_error("Trying to call non-variable expression as a function!", it->context);
                expr->status = Parsing_status::FATAL_ERROR;
                break;
            } else {
                expr = owned_static_cast<Value_expression>(read_function_call(it, owner, std::move(variable_expr), {}));
            }

        } else if (it->type == Token_type::SYMBOL && it->token == "[") {
            ASSERT(false, "indexing NYI");
            // expr = compile_seq_indexing(it, expr);

        } else if (it->type == Token_type::SYMBOL && it->token == ".") {
            expr = read_getter(it, owner, std::move(expr));

        } else if (it->type == Token_type::SYMBOL || it->type == Token_type::IDENTIFIER) {
            // could be an infix opreator. Check if it is.
            // If it isn't, break.
            break; // @TODO: (operators) add support for operators here

            /*
            // Otherwise:
            int op_prio = 0; // FIXME: set correctly
            bool right_associative = false; // FIXME: set correctly
            if (op_prio > min_operator_prio || right_associative && op_prio == min_operator_prio) {
                auto i_expr = Shared<Infix_expr>(new Infix_expr());
                i_expr->owner = owner;
                i_expr->start_token_index = it.current_index;
                i_expr->context = it->context;
                it.eat_token();

                i_expr->lhs = expr;
                i_expr->rhs = compile_value_expression(it, owner, op_prio);
                // FIXME: check for fatal error
                i_expr->rhs->owner = i_expr;
                expr->owner = i_expr;
                expr = std::static_pointer_cast<Value_expression>(i_expr);
            } else {
                break;
            }
            */

        } else {
            break;
        }
    }


    ASSERT(expr != nullptr);
    ASSERT(is_error(expr->status) || expr->status == Parsing_status::FULLY_RESOLVED || expr->status == Parsing_status::DEPENDENCIES_NEEDED);
    return expr;
}


Owned<Variable_expression> read_variable_expression(Token_iterator& it, Shared<Abstx_node> owner, int min_operator_prio)
{
    // since all variable expressions are also value expressions, we can just read it with read_value_expression
    Owned<Value_expression> val_expr = read_value_expression(it, owner, min_operator_prio);
    if (val_expr == nullptr) return nullptr;
    Owned<Variable_expression> var_expr = owned_dynamic_cast<Variable_expression>(std::move(val_expr));
    if (val_expr == nullptr) {
        log_error("Pure value expression used as a variable", val_expr->context);
    }
    return var_expr;
}



/*
    struct {
        a, b : int = 2;
    }
*/
Owned<Value_expression> read_struct_literal(Token_iterator& it, Shared<Abstx_node> owner) {
    Owned<Abstx_struct_literal> o = alloc(Abstx_struct_literal());
    o->owner = owner;
    o->context = it->context;
    o->start_token_index = it.current_index;

    it.assert(Token_type::KEYWORD, "struct");
    it.expect(Token_type::SYMBOL, "{");
    if (it.expect_failed()) {
        add_note("In struct definition here", o->context);
        o->status = Parsing_status::SYNTAX_ERROR;
        return owned_static_cast<Value_expression>(std::move(o));
    }

    // make a temporary scope for struct identifiers
    // these identifiers will then be extracted and added to the struct itself
    o->struct_scope = alloc(Abstx_scope(SCOPE_DYNAMIC)); // set dynamic to make declaration statements try to fully parse immediately
    o->struct_scope->owner = owner;
    o->struct_scope->status = Parsing_status::PARTIALLY_PARSED;
    Seq<size_t> using_indeces;

    // read declaration statments until }
    for (size_t decl_index; !it.compare(Token_type::SYMBOL, "}"); ++decl_index)
    {
        if (it.compare(Token_type::KEYWORD, "using")) {
            it.eat_token();
            using_indeces.add(decl_index);
        }

        // read declaration statement (it is imported to the o->struct_scope automatically)
        Parsing_status status = read_declaration_statement(it, o->struct_scope);
        if (is_error(status)) {
            add_note("In struct definition here", o->context);
            o->status = status;
            if (status == Parsing_status::FATAL_ERROR) {
                // can't continue
                return owned_static_cast<Value_expression>(std::move(o));
            }
        } else if (status == Parsing_status::DEPENDENCIES_NEEDED && !is_error(o->status)) {
            // no error message
            o->status = status;
        }
    }
    it.assert(Token_type::SYMBOL, "}");

    // stop if error
    if (is_error(o->status)) return owned_static_cast<Value_expression>(std::move(o));

    // create struct type
    Owned<CB_Struct> struct_type = alloc(CB_Struct());

    // import identifiers from scope
    size_t using_index_index = 0;
    for (int i = 0; i < o->struct_scope->statements.size; ++i) {
        Shared<Abstx_declaration> decl = dynamic_pointer_cast<Abstx_declaration>(o->struct_scope->statements[i]);
        ASSERT(decl && !is_error(decl->status)); // if not, we should have stopped earlier

        bool is_using(using_indeces[using_index_index] == i);
        if (is_using) ++using_index_index;
        for (const auto& id : decl->identifiers) {
            struct_type->add_member(Shared<Abstx_identifier>(id), is_using);
            // @TODO: if is_using and id is not of a struct type, give warning (using is only relevant for structs containing structs)
        }
    }

    ASSERT(struct_type);
    struct_type->finalize();
    o->struct_type = add_complex_cb_type(owned_static_cast<CB_Type>(std::move(struct_type)));
    o->finalize();
    o->struct_scope->status = o->status; // @check if this shouldn't be inside o->finalize()

    return owned_static_cast<Value_expression>(std::move(o));
}



Owned<Value_expression> read_identifier_reference(Token_iterator& it, Shared<Abstx_node> owner) {
    ASSERT(it->type == Token_type::IDENTIFIER);
    Owned<Abstx_identifier_reference> o = alloc(Abstx_identifier_reference());
    o->owner = owner; // temporary owner; the literal should be owned by a statement somewhere
    o->context = it->context;
    o->start_token_index = it.current_index;
    o->name = it.eat_token().token;

    // try to finalize the token - if not successful status should be DEPENDENCIES_NEEDED
    o->finalize();
    return owned_static_cast<Value_expression>(std::move(o));
}



Owned<Value_expression> read_getter(Token_iterator& it, Shared<Abstx_node> owner, Owned<Value_expression>&& id) {
    const Token_context& dot_context = it->context; // set context to the '.' token (save for later)
    it.assert(Token_type::SYMBOL, "."); // assert and eat the '.' token
    it.expect_current(Token_type::IDENTIFIER);
    if (it.expect_failed()) {
        // @TODO: what should we do?
        // @TODO: set syntax error
    }

    // Get the type of id
    ASSERT(id); // id must exist
    Shared<const CB_Type> id_type = id->get_type();
    ASSERT(id_type); // type must be known

    Shared<const CB_Struct> struct_type = dynamic_pointer_cast<const CB_Struct>(id_type);
    if (struct_type != nullptr) {
        Shared<const CB_Struct::Struct_member> member = struct_type->get_member(it->token);
        if (member != nullptr) {
            // @TODO: create struct member reference abstx node and return it
            Owned<Abstx_struct_getter> o = alloc(Abstx_struct_getter());
            o->owner = owner;
            o->context = dot_context;
            o->start_token_index = it.current_index-1; // pointing to the dot
            id->set_owner(o);
            o->struct_expr = owned_static_cast<Variable_expression>(std::move(id));
            ASSERT(o->struct_expr != nullptr); // Anything with a struct type must be a variable expression (right?)
            o->member = member;
            o->member_id = it->token;
            o->status = Parsing_status::PARTIALLY_PARSED;
            o->finalize();
            it.eat_token(); // eat the identifier token
            return owned_static_cast<Value_expression>(std::move(o));
        }
    }
    // failed to get struct member -> check if it's a dot function call instead

    // check parent scope for a function with the name
    // check if the next token is '('
    // read function call with the id as the first argument
    Owned<Value_expression> id_reference = read_identifier_reference(it, owner);
    if (it.compare(Token_type::SYMBOL, "(")) {
        return owned_static_cast<Value_expression>(read_function_call(it, owner, owned_static_cast<Variable_expression>(std::move(id_reference)), {}, std::move(id)));
    } else {
        log_error("Dot notation used for something that is neither a struct member or a function call", dot_context);
        id_reference->status = Parsing_status::SYNTAX_ERROR;
        return id_reference; // probably best alternative; but this could be nullptr
    }
}





Owned<Value_expression> read_simple_literal(Token_iterator& it, Shared<Abstx_node> owner) {
    Owned<Abstx_simple_literal> o = alloc(Abstx_simple_literal());
    o->owner = owner; // temporary owner; the literal should be owned by a statement somewhere
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
            ASSERT(t.token == "true" || t.token == "false"); // this should be checked by lexer
            // boolean literal
            o->value.v_type = CB_Bool::type;
            o->value.v_ptr = alloc_constant_data(CB_Bool::type->cb_sizeof());
            *(CB_Bool::c_typedef*)o->value.v_ptr = (t.token == "true");
            break;
        case Token_type::STRING:
            // string literal
            o->value.v_type = CB_String::type;
            o->value.v_ptr = alloc_constant_data(CB_String::type->cb_sizeof());
            *(CB_String::c_typedef*)o->value.v_ptr = (CB_String::c_typedef)t.token.c_str(); // valid as long as the list of tokens is alive @warning potentially dangerous
            break;
        default:
            ASSERT(false); // any other type of token cannot be a simple literal
    }

    o->finalize(); // try to finalize the expression
    return owned_static_cast<Value_expression>(std::move(o));
}




Owned<Value_expression> read_sequence_literal(Token_iterator& it, Shared<Abstx_node> owner) {
    // syntax:
    //   [val_expr, val_expr, val_expr] // type of first value determines type (next token is ',' or ']')
    //   [val_expr: val_expr, val_expr] // first value determines type (next token is ':')
    //   [] // type error: unable to determine type of sequence
    //   [int:] // ok

    // @todo

/*
    Parsing_status fully_parse() override {
        if (is_codegen_ready(status)) return status;
        for (auto& v : value) {
            if (is_error(v->fully_parse())) {
                status = v->status;
                return status;
            }
        }
        status = Parsing_status::FULLY_RESOLVED;
        return status;
    }

    Parsing_status finalize() {
        // @todo this should be a part of read_sequence_literal
        if (is_codegen_ready(status)) return status;
        get_type();
        ASSERT(member_type);
        ASSERT(seq_type);
        for (const auto& v : value) {
            if (!is_codegen_ready(v->finalize())) {
                status = v->status;
                return status;
            }
            if (*v->get_type() != *member_type) {
                log_error("Sequence literal member type " + v->get_type()->toS() + " doesn't match the type of the sequence", v->context);
                add_note("Sequence starting here is of type " + seq_type->toS(), context);
                status == Parsing_status::TYPE_ERROR;
                return status;
            }
        }

        status = Parsing_status::FULLY_RESOLVED;
        return status;
    }
*/

    ASSERT(false, "seq literal NYI");
    return nullptr;
}






Owned<Variable_expression> read_function_call(Token_iterator& it, Shared<Abstx_node> owner, Owned<Variable_expression>&& fn_id, const Seq<Shared<Variable_expression>>& lhs, Owned<Value_expression>&& first_arg) {
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
    o->owner = owner; // temporary
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
    else if (fn_type != nullptr && fn_type->out_types.size > 0) {
        // create declaration statement with tmp variables; push it to scope
        // add its temporary variables as out_args
        Owned<Abstx_declaration> tmp_decl = alloc(Abstx_declaration());
        tmp_decl->owner = owner;
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
        tmp_decl->status = Parsing_status::FULLY_RESOLVED; // mark as resolved @check if some errors should be reported here
        o->parent_scope()->statements.add(owned_static_cast<Statement>(std::move(tmp_decl)));
    }

    it.assert(Token_type::SYMBOL, "("); // this should already have been checked

    // add first arg (if applicable; only for dot call syntax)
    if (first_arg != nullptr) {
        first_arg->set_owner(o);
        o->in_args.add(std::move(first_arg));
    }

    // add arguments given in parens
    while (1) {
        if (it.compare(Token_type::SYMBOL, ")")) break; // done

        Owned<Value_expression> arg = read_value_expression(it, owner);
        ASSERT(arg != nullptr);
        arg->set_owner(o);
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
        o->status = Parsing_status::FULLY_RESOLVED;
        it.assert(Token_type::SYMBOL, ")"); // this is already checked / now eat the token
    } else if (!is_fatal(o->status)) {
        it.current_index = it.find_matching_paren() + 1;
    }
    if (it.expect_failed()) {
        o->status = Parsing_status::FATAL_ERROR;
    }

    // create a function call expression to reference the function call statement
    Owned<Abstx_function_call_expression> expr = alloc(Abstx_function_call_expression());
    expr->owner = owner;
    expr->context = o->context;
    expr->start_token_index = o->start_token_index;
    expr->function_call = o;
    expr->status = o->status;
    expr->finalize();

    // add the function call statement to parent scope as a separate statement
    o->parent_scope()->statements.add(owned_static_cast<Statement>(std::move(o)));

    // return the function call expression
    return owned_static_cast<Variable_expression>(std::move(expr));
}

Parsing_status Abstx_function_call::fully_parse() {
    // Abstx_function_call is just an imaginary construct with no token context
    // It should always be finished during read_function_call()
    ASSERT(is_error(status) || is_codegen_ready(status));
    return status;
}





/*
Parsing_status Abstx_function::finalize() {
    // @TODO: This should be done as a part of read_function_expression

    if (is_codegen_ready(status)) return status;
    ASSERT(function_identifier != nullptr);

    // check function identifier
    if (!is_codegen_ready(function_identifier->status)) {
        if (is_error(function_identifier->status)) {
            // error in dependency -> this inherits the error
            status = function_identifier->status;
        } else {
            // dependency is just not finished parsing yet - wait for it and try again later
            // @todo add dependency chain
            status = Parsing_status::DEPENDENCIES_NEEDED;
        }
        return status;
    }

    // type is finalized -> get a pointer to it so we can typecheck arguments
    Shared<const CB_Function> fn_type = dynamic_pointer_cast<const CB_Function>(function_identifier->get_type());
    ASSERT(fn_type != nullptr);

    // check function scope. This also finalizes all argument identifiers
    if (!is_codegen_ready(scope->finalize())) {
        status = scope->status;
        return status;
    }

    // check arguments (the same way as function_identifier, but also check type)
    if (in_args.size != fn_type->in_types.size) {
        log_error("type mismatch in function in types: wrong number of in parameters", context);
        status = Parsing_status::TYPE_ERROR;
        return status;
    }
    size_t index = 0;
    for (const auto& fa : in_args) {
        if (!is_codegen_ready(fa->identifier->status)) {
            if (is_error(fa->identifier->status)) {
                status = fa->identifier->status;
            } else {
                // @todo add dependency chain
                status = Parsing_status::DEPENDENCIES_NEEDED;
            }
            return status;
        }
        if (*fa->identifier->get_type() != *fn_type->in_types[index]) {
            log_error("type mismatch in function in types", context);
            add_note("argument is of type " + fa->identifier->get_type()->toS() + ", but the function type expected type " + fn_type->in_types[index]->toS());
            status = Parsing_status::TYPE_ERROR;
            return status;
        }
        index++;
    }

    if (out_args.size != fn_type->out_types.size) {
        log_error("type mismatch in function out types: wrong number of out parameters", context);
        status = Parsing_status::TYPE_ERROR;
        return status;
    }
    index = 0;
    for (const auto& fa : out_args) {
        if (!is_codegen_ready(fa->identifier->status)) {
            if (is_error(fa->identifier->status)) {
                status = fa->identifier->status;
            } else {
                // @todo add dependency chain
                status = Parsing_status::DEPENDENCIES_NEEDED;
            }
            return status;
        }
        if (*fa->identifier->get_type() != *fn_type->out_types[index]) {
            log_error("type mismatch in function out types", context); // @todo: write better error message, including which types are mismatching
            add_note("argument is of type " + fa->identifier->get_type()->toS() + ", but the function type expected type " + fn_type->in_types[index]->toS());
            status = Parsing_status::TYPE_ERROR;
            return status;
        }
        index++;
    }

    // we reached the end -> we are done
    status = Parsing_status::FULLY_RESOLVED;
    return status;
}
*/



void read_function_arguments(Token_iterator& it, Shared<Abstx_function_literal> fn, bool in, bool allow_multiple_args) {
    ASSERT(fn);

    while(1) {
        Abstx_function_literal::Function_arg arg{};
        Owned<Abstx_identifier> id = alloc(Abstx_identifier());
        id->set_owner(Shared<Abstx_node>(&fn->scope));
        id->context = it->context;
        id->start_token_index = it.current_index;

        if (it.compare(Token_type::SYMBOL, "$")) {
            arg.generic_id = true;
            it.eat_token();
        }

        // create an identifier
        id->name = it.expect(Token_type::IDENTIFIER).token;
        if (it.expect_failed()) break; // syntax error

        it.expect(Token_type::SYMBOL, ":");
        if (it.expect_failed()) break; // syntax error

        if (it.compare(Token_type::SYMBOL, ":") || it.compare(Token_type::SYMBOL, "=")) {
            ASSERT(false, "Default values for function arguments not yet implemented");
        }

        if (it.compare(Token_type::SYMBOL, "$")) {
            arg.generic_type = true;
            it.eat_token();
            const Token& type_token = it.expect(Token_type::IDENTIFIER);
            // @todo what to do with this type name?
            ASSERT(false, "generic types NYI");
        } else {
            // read regular type expression
            Owned<Value_expression> type_expr = read_value_expression(it, static_pointer_cast<Abstx_node>(fn));
            ASSERT(type_expr);
            if (is_error(type_expr->status) || type_expr->status == Parsing_status::DEPENDENCIES_NEEDED) {
                id->status = type_expr->status;
            } else {
                Shared<const CB_Type> type_type = type_expr->get_type();
                if (*type_type != *CB_Type::type) {
                    log_error("Non-type expresson used as type", type_expr->context);
                    add_note("In function argument here", fn->context);
                    id->status = Parsing_status::TYPE_ERROR;
                } else {
                    ASSERT(type_expr->has_constant_value());
                    id->value.v_type = parse_type(type_expr->get_constant_value());
                }
            }
        }
        if (is_error(id->status)) {
            fn->status = id->status;
            break;
        }

        // add the identifier and arg to the function
        arg.identifier = id;
        fn->scope.fn_identifiers[id->name] = std::move(id);
        if (in) fn->in_args.add(std::move(arg));
        else fn->out_args.add(std::move(arg));

        if (it.compare(Token_type::SYMBOL, ":") || it.compare(Token_type::SYMBOL, "=")) {
            ASSERT(false, "Default values for function arguments not yet implemented");
        }

        // @TODO: actually add the argument to the function / function scope


        if (allow_multiple_args && it.compare(Token_type::SYMBOL, ",")) {
            it.eat_token(); // eat the token and read another argument
        } else {
            // unable to read more arguments
            return;
        }
    }

}


Owned<Value_expression> read_function_literal(Token_iterator& it, Shared<Abstx_node> owner)
{
    std::cout << "reading function literal" << std::endl;
    Owned<Abstx_function_literal> o = alloc(Abstx_function_literal());
    o->owner = owner;
    o->context = it->context;
    o->start_token_index = it.current_index;

    std::cout << "initializing fn scope" << std::endl;
    o->scope.set_owner(o);
    o->scope.flags += SCOPE_DYNAMIC;
    o->scope.status = Parsing_status::NOT_PARSED;

    std::cout << "initializing fn id" << std::endl;
    o->function_identifier.set_owner(o);
    o->function_identifier.value_expression = static_pointer_cast<Value_expression>(o);
    o->function_identifier.name = "_cb_fn";
    o->function_identifier.uid = get_unique_id();
    o->function_identifier.context = it->context;
    o->function_identifier.start_token_index = it.current_index;
    o->function_identifier.status = Parsing_status::NOT_PARSED;

    it.assert(Token_type::KEYWORD, "fn"); // eat the "fn" token
    it.expect(Token_type::SYMBOL, "(");
    if (it.expect_failed()) {
        o->status = Parsing_status::SYNTAX_ERROR;
        return owned_static_cast<Value_expression>(std::move(o));
    }

    if (!it.compare(Token_type::SYMBOL, ")")) {
        std::cout << "reading in arguments" << std::endl;
        // read in arguments
        read_function_arguments(it, o, true, true); // in arguments

        // check for errors
        if (it.expect_failed()) {
            o->status = Parsing_status::SYNTAX_ERROR;
        }
        if (is_error(o->status)) return owned_static_cast<Value_expression>(std::move(o));
    }
    // end of in arguments
    it.expect(Token_type::SYMBOL, ")");

    if (it.compare(Token_type::SYMBOL, "->")) {
        std::cout << "reading out arguments" << std::endl;
        // read out arguments
        it.eat_token(); // eat the "->" token
        bool parens = false;
        if (it.compare(Token_type::SYMBOL, "(")) {
            it.eat_token();
            parens = true;
        }
        read_function_arguments(it, o, false, parens); // in arguments

        if (parens) {
            it.expect(Token_type::SYMBOL, ")");
        }

        // check for errors
        if (it.expect_failed()) {
            o->status = Parsing_status::SYNTAX_ERROR;
        }
        if (is_error(o->status)) return owned_static_cast<Value_expression>(std::move(o));
    }

    // set scope context
    o->scope.context = it->context;
    o->scope.start_token_index = it.current_index;

    it.expect(Token_type::SYMBOL, "{");
    if (it.expect_failed()) {
        o->status = Parsing_status::SYNTAX_ERROR;
    }
    if (!is_error(o->status)) o->status = Parsing_status::FULLY_RESOLVED; // @check should this be FULLY_RESOLVED?

    // @todo: before reading statements, ensure that the function type is ready to use
    // @todo: (recursive functions) this function must return first so the value expression can be used
    o->finalize(); // finalize before read_scope_statements()

    read_scope_statements(it, &o->scope);

    if (!is_fatal(o->scope.status)) {
        it.expect(Token_type::SYMBOL, "}");
        if (it.expect_failed()) o->status == Parsing_status::FATAL_ERROR;
    } else {
        o->status = Parsing_status::FATAL_ERROR;
    }

    std::cout << "done reading function literal!" << std::endl;

    return owned_static_cast<Value_expression>(std::move(o));
}

void Abstx_function_literal::finalize()
{
    // note: function scope doesn't need to be fully parsed for the function literal to be
    if (is_error(status)) return;
    if (is_codegen_ready(status) && is_codegen_ready(scope.status)) return;
    std::cout << "finalizing Abstx_function_literal" << std::endl;

    ASSERT(function_identifier.name != ""); // must be set during creating
    if (function_identifier.value.v_type == nullptr) {
        // finalize all identifiers and build function type
        Owned<CB_Function> fn_type = alloc(CB_Function()); // owned by some global list of types

        for (const auto& arg : in_args) {
            arg.identifier->finalize();
            if (is_error(arg.identifier->status)) {
                status = arg.identifier->status;
                std::cout << "unable to finalize in argument" << std::endl;
                return;
            } else if (arg.identifier->status == Parsing_status::DEPENDENCIES_NEEDED) {
                status = Parsing_status::DEPENDENCIES_NEEDED;
            }
            Shared<const CB_Type> type = arg.identifier->get_type();
            ASSERT(type);
            fn_type->in_types.add(type);
        }

        for (const auto& arg : out_args) {
            arg.identifier->finalize();
            if (is_error(arg.identifier->status)) {
                status = arg.identifier->status;
                std::cout << "unable to finalize out argument" << std::endl;
                return;
            } else if (arg.identifier->status == Parsing_status::DEPENDENCIES_NEEDED) {
                status = Parsing_status::DEPENDENCIES_NEEDED;
            }
            Shared<const CB_Type> type = arg.identifier->get_type();
            ASSERT(type);
            fn_type->out_types.add(type);
        }

        function_identifier.value.v_type = static_pointer_cast<const CB_Type>(add_complex_type(std::move(fn_type)));
    }
    ASSERT(function_identifier.value.v_type); // if type creation failed we should have already returned
    function_identifier.value.v_ptr = this;
    function_identifier.status = Parsing_status::FULLY_RESOLVED;

    // update status
    std::cout << "successfully finalizing Abstx_function_literal" << std::endl;
    if (!is_error(status) && status != Parsing_status::DEPENDENCIES_NEEDED) status = Parsing_status::FULLY_RESOLVED;
}





Owned<Value_expression> read_function_type(Token_iterator& it, Shared<Abstx_node> owner)
{
    it.assert(Token_type::KEYWORD, "fn"); // eat the "fn" token
    ASSERT(false, "NYI");
}


// fn() {}
// fn() {}
// fn()->() {}
// fn()->() {}
// fn()->() {}
Owned<Value_expression> read_fn_literal(Token_iterator& it, Shared<Abstx_node> owner)
{
    int start_index = it.current_index; // save this for later
    it.assert(Token_type::KEYWORD, "fn"); // eat the "fn" token

    // first: find if the function has {}
    while (!it.error) {

        const Token& t = it.eat_token();

        // look for ';' (end of statement -> fn type literal), or '{' (function scope start)
        // the first such matching symbol found determines the type of expression.
        if (t.type == Token_type::SYMBOL) {

            if (t.token == ";") {
                // end of statement without finding a function scope -> its just a type literal
                it.current_index = start_index;
                return read_function_type(it, owner);
            }

            else if (t.token == "{") {
                // function scope -> it's a function literal
                it.current_index = start_index;
                return read_function_literal(it, owner);
            }

            else if (t.token == "(") it.current_index = it.find_matching_paren(it.current_index-1) + 1;
            else if (t.token == "[") it.current_index = it.find_matching_bracket(it.current_index-1) + 1;
            else if (t.token == "{") it.current_index = it.find_matching_brace(it.current_index-1) + 1;

            else if (t.is_eof() || t.token == ")" || t.token == "]" || t.token == "}") {
                // unable to find a proper statement
                log_error("Unexpected token in function expression: \""+t.token+"\"", t.context);
                it.error = true;
                break;
            }
        }
    }
    ASSERT(it.error);
    add_note("In function expression that started here: ", it.look_at(start_index).context);

    // instead of returning nullpointer, call read_function_type and let it handle more specific error messages
    // it should end in fatal error
    // then just return it
    it.current_index = start_index;
    auto p = read_function_type(it, owner);
    ASSERT(p->status == Parsing_status::FATAL_ERROR);
    return p;
}



/*
Parsing_status Abstx_identifier_reference::finalize() {
    // TODO: this should be in read_expression
    if (is_codegen_ready(status) || is_error(status)) return status;
    id = parent_scope()->get_identifier(name);
    if (id == nullptr) {
        log_error("Use of undefined identifier", context);
        status = Parsing_status::UNDECLARED_IDENTIFIER;
    } else {
        status = Parsing_status::FULLY_RESOLVED;
    }
    return status;
}
*/





















/*
const int eval_prio = 1000; // FIXME: fix. Maybe use the current operator prio?


Shared<Literal> compile_eval_expr(Token_iterator& it, Shared<Abstx_node> owner)
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








Shared<Value_expression> compile_value_list(Token_iterator& it, Shared<Abstx_node> owner)
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










Shared<Variable_expression> compile_variable_expression(Token_iterator& it, Shared<Abstx_node> owner)
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



