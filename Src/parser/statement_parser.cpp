#include "parser.h"
// all types of statements are needed for static casts
#include "../abstx/statements/abstx_assignment.h"
#include "../abstx/statements/abstx_c_code.h"
#include "../abstx/statements/abstx_declaration.h"
#include "../abstx/statements/abstx_defer.h"
#include "../abstx/statements/abstx_for.h"
#include "../abstx/statements/abstx_function_call.h"
#include "../abstx/statements/abstx_if.h"
#include "../abstx/statements/abstx_return.h"
#include "../abstx/statements/abstx_using.h"
#include "../abstx/statements/abstx_while.h"
#include "../abstx/abstx_scope.h"


// --------------------------------------------------------------------------------------------------------------------
//
//          Statement examination
//          Used to determine which type of statement to read
//          Also used to verify that the general code structure is valid, e.g. that parens
//            match and that each statement is ended with a semicolon (if applicable)
//
// --------------------------------------------------------------------------------------------------------------------



// Perform partial parsing of a statement, adding the parsed statements to the parent scope in the process
// If the scope is dynamic, all statements has to be fully parsed and finalized immediately
// Any function calls should be added as separate statements before current statement
// Returns the parsing status of the read statement
Parsing_status read_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope)
{
    ASSERT(parent_scope != nullptr);

    if (it->is_eof() || it.compare(Token_type::SYMBOL, "}")) {
        // end of scope -> no error, just return
        return Parsing_status::NOT_PARSED;

    } else if (it.compare(Token_type::SYMBOL, ";")) {
        // empty statement, give warning and ignore
        log_warning("Additional ';' found", it->context);
        it.eat_token(); // eat the ';' token
        return read_statement(it, parent_scope);

    } else if (it.compare(Token_type::SYMBOL, "{")) {
        // nested scope
        return read_anonymous_scope(it, parent_scope);

    } else if (it.compare(Token_type::KEYWORD, "if")) {
        // if statement
        return read_if_statement(it, parent_scope);

    } else if (it.compare(Token_type::KEYWORD, "for")) {
        // for statement
        return read_for_statement(it, parent_scope);

    } else if (it.compare(Token_type::KEYWORD, "while")) {
        // while statement
        return read_while_statement(it, parent_scope);

    } else if (it.compare(Token_type::KEYWORD, "return")) {
        // return statement
        return read_return_statement(it, parent_scope);

    } else if (it.compare(Token_type::KEYWORD, "defer")) {
        // defer statement
        return read_defer_statement(it, parent_scope);

    } else if (it.compare(Token_type::KEYWORD, "using")) {
        // using statement
        return read_using_statement(it, parent_scope);

    } else if (it.compare(Token_type::COMPILER_COMMAND, "#c")) {
        // c code statement
        return read_c_code_statement(it, parent_scope);

    } else if (it.compare(Token_type::COMPILER_COMMAND, "#run")) {
        // the next token must be the start of a function call expression
        auto s = read_run_expression(it, parent_scope);
        if (s != nullptr) {
            it.expect_end_of_statement();
            if (it.expect_failed()) {
                add_note("In #run statement that started here", s->context);
                s->status = Parsing_status::FATAL_ERROR;
            }
        }
        return s->status;

    } else {

        int start_index = it.current_index;
        Token_context context = it->context;

        while (!it.error && !it->is_eof()) {

            // look for ':' (declaration), '=' (assignment) and ';' (unknown statement)
            // the first such matching symbol found determines the type of statement.

            const Token& t = it.eat_token();

            if (t.type == Token_type::SYMBOL) {

                if (t.token == ":") {
                    // declaration statement
                    it.current_index = start_index;
                    return read_declaration_statement(it, parent_scope);
                }

                else if (t.token == "=") {
                    // assignment statement
                    it.current_index = start_index;
                    return read_assignment_statement(it, parent_scope);
                }

                else if (t.token == ";") {
                    // its a statement with only a value expression in it
                    it.current_index = start_index;
                    return read_value_statement(it, parent_scope);
                }

                else if (t.token == "(") it.current_index = it.find_matching_paren(it.current_index-1) + 1; // go back to the previous "(" and search from there
                else if (t.token == "[") it.current_index = it.find_matching_bracket(it.current_index-1) + 1;
                else if (t.token == "{") it.current_index = it.find_matching_brace(it.current_index-1) + 1;

                else if (t.is_eof() || t.token == ")" || t.token == "]" || t.token == "}") {
                    // unable to find a proper statement
                    log_error("Missing ';' at the end of statement: expected \";\" before \""+t.token+"\"", t.context);
                    add_note("In unresolved statement here", it.look_at(start_index).context);
                    return Parsing_status::FATAL_ERROR;
                }
            }
        }
    }

    ASSERT(false); // should never be reached
    return Parsing_status::NOT_PARSED;
}



Parsing_status read_scope_statements(Token_iterator& it, Shared<Abstx_scope> scope) {

    bool brace_enclosed = it.eat_conditonal(Token_type::SYMBOL, "{"); // global scopes are not brace enclosed

    Parsing_status status = Parsing_status::NOT_PARSED;
    while (!it.compare(Token_type::SYMBOL, "}") && !is_eof(it->type)) {
        status = read_statement(it, scope);
        ASSERT(!(scope->dynamic() && status == Parsing_status::DEPENDENCIES_NEEDED)); // DEPENDENCIES_NEEDED not allowd in dynamic scopes - everything should be verifiable immedately; otherwise there should be an error
        // add_note("read statement with status "+toS(status)+", now it is here", it->context); // @debug
        // fatal error -> give up
        // other error -> failed to parse dynamic scope, but we can continue anyway to get more error messages
        if (is_error(status)) {
            scope->status = status;
            if (is_fatal(scope->status)) return scope->status; // give up
        }
    }

    if (brace_enclosed) {
        if (is_eof(it->type)) {
            log_error("Unexpected end of file in the middle of a scope", it->context);
            add_note("In scope that started here", scope->context);
            scope->status = Parsing_status::FATAL_ERROR;
        }

        if (!is_fatal(scope->status)) {
            it.expect(Token_type::SYMBOL, "}"); // we should have found this already. Now eat it so we return with it pointing to after the brace
            if (it.expect_failed()) {
                add_note("In scope that started here", scope->context);
                scope->status = Parsing_status::FATAL_ERROR;
            }
        }
    } else if (!is_eof(it->type)) {
        log_error("Unexpected closing brace in the middle of a scope", it->context);
        add_note("In scope that started here", scope->context);
        scope->status = Parsing_status::FATAL_ERROR;
    }

    if (!is_error(scope->status)) {
        if (scope->dynamic()) {
            scope->status = Parsing_status::FULLY_RESOLVED;
        } else {
            scope->status = Parsing_status::PARTIALLY_PARSED;
        }
    }

    return scope->status;
}

// syntax:
// { }
Owned<Abstx_scope> read_scope(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    // @todo: if dynamic: fully resolve statements immediately
    // @todo: write function continue_parse_scope() that can finish parsing a scope that is not fully parsed

    it.assert_current(Token_type::SYMBOL, "{"); // checked previously

    // create scope
    Owned<Abstx_scope> scope = alloc(Abstx_scope(parent_scope->flags));
    scope->set_owner(parent_scope);
    scope->context = it->context;
    scope->start_token_index = it.current_index;

    if (scope->dynamic()) {
        // parse and resolve all statements in the scope
        read_scope_statements(it, scope);
    } else {
        // just find closing brace / don't read statements
        it.current_index = it.find_matching_brace() + 1;
        if (it.expect_failed()) {
            scope->status = Parsing_status::FATAL_ERROR;
        }
    }

    if (!is_error(scope->status)) scope->status = Parsing_status::PARTIALLY_PARSED;

    return scope;
}



// syntax (dynamic scope only):
// { }
Parsing_status read_anonymous_scope(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    it.assert_current(Token_type::SYMBOL, "{"); // checked previously

    Owned<Abstx_anonymous_scope> o = alloc(Abstx_anonymous_scope()); // owned pointer, this will be destroyed when we move it into the scopes list of statments
    Shared<Abstx_anonymous_scope> s = o; // shared pointer that we can use and modify however we like
    s->set_owner(parent_scope);
    s->context = it->context;
    s->start_token_index = it.current_index;

    if (!parent_scope->dynamic()) {
        // only allowed in dynamic context
        log_error("Anonymous scope used in static context. That makes no sense!", it->context);
        s->status = Parsing_status::SYNTAX_ERROR;
        it.current_index = it.find_matching_brace() + 1; // go past the } token
        if (it.expect_failed()) s->status = Parsing_status::FATAL_ERROR;

    } else {
        // read scope
        s->scope = read_scope(it, parent_scope);
        ASSERT(s->scope != nullptr);
        // s->scope->set_owner(s); // no need to update owner; parent_scope is good enough
        ASSERT(is_error(s->scope->status) || s->scope->status == Parsing_status::DEPENDENCIES_NEEDED || s->scope->status == Parsing_status::FULLY_RESOLVED);

        s->status = s->scope->status;
        parent_scope->statements.add(std::move(owned_static_cast<Statement>(std::move(o))));
        ASSERT(o == nullptr);
    }
    ASSERT(s != nullptr);
    return s->status;
}




// read_value_rhs used for Abstx_declaration and Abstx_assignment
Parsing_status read_value_rhs(Token_iterator& it, Shared<Abstx_node> owner, Seq<Owned<Value_expression>>& expressions, bool constant) {
    Parsing_status status = Parsing_status::NOT_PARSED;
    do {
        Owned<Value_expression> value_expr = read_value_expression(it, owner);
        if (value_expr == nullptr) {
            status = Parsing_status::SYNTAX_ERROR; // unable to read value expression
            return status; // give up
        }
        if (is_error(value_expr->status) || (!is_error(status) && value_expr->status == Parsing_status::DEPENDENCIES_NEEDED)) {
            status = value_expr->status;
            if (is_fatal(status)) break; // give up

        } else if (Shared<Abstx_function_call_expression> fn_call = dynamic_pointer_cast<Abstx_function_call_expression>(value_expr)) {
            // add it's out arguments, then set the expression to null to indicate we are done
            for (const auto& arg : fn_call->function_call->out_args) {
                Owned<Variable_expression_reference> ref = alloc(Variable_expression_reference());
                ref->owner = owner;
                ref->set_reference(arg);
                ref->finalize();
                if(constant && !ref->has_constant_value()) {
                    log_error("Unable to declare a constant value from a non-constant value expression", value_expr->context);
                    ref->status = Parsing_status::COMPILE_TIME_ERROR;
                    status = Parsing_status::COMPILE_TIME_ERROR;
                }
                expressions.add(owned_static_cast<Value_expression>(std::move(ref)));
            }
            value_expr = nullptr;
        } else {
            ASSERT(value_expr->get_type() != nullptr, "Value expression should have error status if it can't infer type after read_value_expression()");
            if(constant && !value_expr->has_constant_value()) {
                log_error("Unable to declare a constant value from a non-constant value expression", value_expr->context);
                value_expr->status = Parsing_status::COMPILE_TIME_ERROR;
                status = Parsing_status::COMPILE_TIME_ERROR;
            }
        }

        if (value_expr) {
            expressions.add(std::move(value_expr));
        }

    } while (it.eat_conditonal(Token_type::SYMBOL, ","));
    return status;
}


// syntax:
// a, b, c := expr;
// a, b, c : expr = expr;
// after a successful read, s->start_token_index will point to the first token after ':'.
Parsing_status read_declaration_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    // we know that there are a ':' somewhere, so just read identifiers until we reach it

    // expect identifier token
    // optional, read ',' then expect another identifier token
    // expect ':' token
    // find end of statement (';')

    // @todo operator declaration - they have a different identifier syntax

    // LOG("reading declaration statement at " << it->context.toS());

    Owned<Abstx_declaration> o = alloc(Abstx_declaration()); // will be destroyed later
    Shared<Abstx_declaration> s = o; // shared pointer that we can use and modify however we like
    s->set_owner(parent_scope);
    s->context = it->context;
    s->start_token_index = it.current_index;

    // TODO: add special check for if the first token is ':'

    // LOG("reading declaration statement");
    do {
        // allocate abstx identifier and set base info
        Owned<Abstx_identifier> id = alloc(Abstx_identifier());
        id->set_owner(s);
        id->context = it->context;
        id->start_token_index = it.current_index;

        // LOG("reading declared identifier starting with token " << it->toS() << " at index " << it.current_index);

        // read identifier token
        if (it.compare(Token_type::KEYWORD, "operator")) {
            // @TODO also accept operator definitions
            // operator(type)op(type)
            // operator op(type)
            // operator (type)op
            log_error("Operator overloading is not implemented yet!", it->context);
        }

        const Token& t = it.expect(Token_type::IDENTIFIER);
        if (it.expect_failed()) {
            s->status = Parsing_status::SYNTAX_ERROR;
            break;
        }

        id->name = t.token;
        // check if there is another id with the same name in the current local scope (not allowed)
        auto old_id = parent_scope->get_identifier(id->name, false);
        if (old_id != nullptr) {
            log_error("Redeclaration of identifier "+id->name, id->context);
            add_note("Previously declared here", old_id->context);
            id->status = Parsing_status::REDECLARED_IDENTIFIER;
            // no specific error in the declaration, just ignore the identifier
        } else {
            // add the identifier to the scope
            parent_scope->identifiers[id->name] = (Shared<Abstx_identifier>)id;
            id->status = Parsing_status::PARTIALLY_PARSED;
        }
        s->identifiers.add(std::move(id));

    } while (it.eat_conditonal(Token_type::SYMBOL, ","));

    s->start_token_index = it.current_index; // update start_token_index to point to the ':' token
    it.expect(Token_type::SYMBOL, ":");
    if (it.expect_failed()) {
        s->status = Parsing_status::SYNTAX_ERROR;
    }

    if (is_error(s->status)) {
        add_note("In declaration statement here", s->context);
    }

    // LOG("done reading declaration");

    // we are done reading -> set the start_token_index to the first value of RHS
    s->start_token_index = it.current_index;

    // find the closing ';'
    // @optimize if dynamic scope, this shouldn't be necessary - fully parse the statement immediately, then check for the ';'
    it.current_index = it.find_matching_semicolon() + 1;
    if (it.expect_failed()) {
        add_note("In declaration statement here", s->context);
        s->status = Parsing_status::FATAL_ERROR;
        return s->status;
    }

    // update status
    if (!is_error(s->status)) s->status = Parsing_status::PARTIALLY_PARSED;

    // if we are in a dynamic scope, we must finalize the statement immediately
    // this might add one or more function call statements to the scope -> we have to do this before adding the declaration statement to the scope
    // (in a static scope the order of statements doesn't matter, so adding the function calls later is okay)
    if (parent_scope->dynamic()) {
        s->fully_parse();
    }

    // add it to the scope
    parent_scope->statements.add(std::move(owned_static_cast<Statement>(std::move(o))));

    return s->status;
}

Parsing_status Abstx_declaration::fully_parse() {
    if (status != Parsing_status::PARTIALLY_PARSED) return status;
    ASSERT(identifiers.size != 0);
    Token_iterator it = global_scope()->iterator(start_token_index); // starting with the first value after the ':' token

    // LOG("fully parsing declaration statement starting with token " << it->toS() << " at index " << it.current_index);

    // @todo: allow constant function calls as type identifiers
    // @todo: allow #run function calls as type identifiers
    // @todo: don't overwrite FATAL_ERROR with different error message
    // @todo: allow function calls as RHS

    if (!it.compare(Token_type::SYMBOL, "=") && !it.compare(Token_type::SYMBOL, ":")) {
        // read list of types
        do {

            Owned<Value_expression> type_expr = read_value_expression(it, this);
            if (type_expr == nullptr) {
                status = Parsing_status::SYNTAX_ERROR; // unable to read value expression
                return status; // give up
            }
            if (is_error(type_expr->status)) {
                add_note("In declaration statement here", context);
                status = type_expr->status;
                return status;

            } else if (type_expr->status == Parsing_status::DEPENDENCIES_NEEDED) {
                LOG("type expression has Parsing_status::DEPENDENCIES_NEEDED");
                status = type_expr->status;
                // continue reading
            } else if (Shared<Abstx_function_call_expression> fn_call = dynamic_pointer_cast<Abstx_function_call_expression>(type_expr)) {
                // @todo: allow function calls if they are constant
                log_error("Function calls not allowd as type specifier", type_expr->context);
                status = Parsing_status::TYPE_ERROR;
                // for completeness sake, add the out arguments, then set the expression to null to indicate we are done
                for (const auto& arg : fn_call->function_call->out_args) {
                    Owned<Variable_expression_reference> ref = alloc(Variable_expression_reference());
                    ref->owner = this;
                    ref->set_reference(arg);
                    ref->finalize();
                    type_expressions.add(owned_static_cast<Value_expression>(std::move(ref)));
                }
                type_expr = nullptr;

            } else {
                Shared<const CB_Type> t = type_expr->get_type();
                ASSERT(t != nullptr, "Value expression should have error status if it can't infer type after read_value_expression()");
                if (*t != *CB_Type::type) {
                    log_error("Non-type expresson used as type in declaration statement", type_expr->context);
                    status = Parsing_status::TYPE_ERROR;
                } else if(!type_expr->has_constant_value()) {
                    log_error("Unable to infer type from type expression at compile time", type_expr->context);
                    add_note("All types must be known at compile time!");
                    status = Parsing_status::TYPE_ERROR;
                }
            }

            if (type_expr) {
                type_expressions.add(std::move(type_expr));
            }

        } while (it.eat_conditonal(Token_type::SYMBOL, ","));

        // LOG("assigning types");

        // list must either be of size 1 (all identifiers have the same type) or the same size as the number of identifiers
        if (type_expressions.size != 1 && type_expressions.size != identifiers.size) {
            log_error("Wrong number of types supplied in declaration statement", context);
            if (identifiers.size == 1) add_note("Expected 1 type but found "+std::to_string(type_expressions.size), context);
            else add_note("Expected either 1 or "+std::to_string(identifiers.size)+" types but found "+std::to_string(type_expressions.size), context);
            status = Parsing_status::SYNTAX_ERROR;
        } else {
            // assign types to identifiers
            size_t index = 0;
            for (const auto& id : identifiers) {
                // @todo (operators) the operator should already have a type - a function type with in arguments defined. Check that the types match and assign out argument types!
                Shared<Value_expression> type_expr = type_expressions[index];
                if (!is_error(type_expr->status) && type_expr->status != Parsing_status::DEPENDENCIES_NEEDED) {
                    id->value.v_type = parse_type(type_expressions[index]->get_constant_value());
                    ASSERT(id->value.v_type != nullptr);
                } else ASSERT(is_error(status) || status == Parsing_status::DEPENDENCIES_NEEDED);
                if (type_expressions.size != 1) ++index;
            }
        }
    }

    if (it.compare(Token_type::SYMBOL, "=") || it.compare(Token_type::SYMBOL, ":")) {
        bool constant = it.compare(Token_type::SYMBOL, ":");
        it.eat_token(); // eat the ':'/'=' token

        // LOG("reading " << (constant?"constant":"non-constant") << " values in declaration");

        // read list of values and check for errors
        Parsing_status rhs_status = read_value_rhs(it, this, value_expressions, constant);
        if (is_error(rhs_status)) {
            status = rhs_status;
            return status;
        }

        // LOG("assigning " << (constant?"constant":"non-constant") << " values");

        // list must either be of size 1 (all identifiers have the same value) or the same size as the number of identifiers
        if (value_expressions.size != 1 && value_expressions.size != identifiers.size) {
            log_error("Wrong number of values supplied in declaration statement", context);
            if (identifiers.size == 1) add_note("Expected 1 type but found "+std::to_string(value_expressions.size), context);
            else add_note("Expected either 1 or "+std::to_string(identifiers.size)+" types but found "+std::to_string(value_expressions.size), context);
            if (!is_error(status)) status = Parsing_status::SYNTAX_ERROR;
        } else {
            // assign values to identifiers
            size_t index = 0;
            for (const auto& id : identifiers) {
                // @todo (operators) the operator should already have a type - a function type with in arguments defined. Check that the types match and assign out argument types!
                Shared<Value_expression> value_expr = value_expressions[index];
                if (constant) {
                    id->value_expression = value_expr;
                    // if not constant, then we cant be sure that the value_expr is the actual value used later (it might be overwritten)
                }

                // check that type match or assign type if it hasn't been inferred yet
                Shared<const CB_Type> type = value_expr->get_type();
                ASSERT(type != nullptr || is_error(value_expr->status) || value_expr->status == Parsing_status::DEPENDENCIES_NEEDED);
                if (type != nullptr) {
                    if (id->value.v_type == nullptr) id->value.v_type = type;
                    else if (*id->value.v_type != *type) {
                        log_error("Type of value expression doesn't match the type of the assigned identifier!", value_expr->context);
                        add_note("Unable to convert type from "+type->toS()+" to "+id->value.v_type->toS());
                        value_expr->status = Parsing_status::TYPE_ERROR;
                        status = Parsing_status::TYPE_ERROR;
                    }
                }

                if (constant) {
                    if (!is_error(value_expr->status) && value_expr->status != Parsing_status::DEPENDENCIES_NEEDED) {
                        ASSERT(value_expr->has_constant_value());
                        const Any& c_value = value_expr->get_constant_value();
                        ASSERT(*c_value.v_type == *id->value.v_type); // checked above
                        id->value.v_ptr = c_value.v_ptr;
                    } else ASSERT(is_error(status) || status == Parsing_status::DEPENDENCIES_NEEDED);
                }

                if (value_expressions.size != 1) ++index;
            }
        }

    }

    // LOG("end of declaration statement");

    it.expect_end_of_statement();
    if (it.expect_failed()) {
        add_note("In declaration statement here", context);
        status = Parsing_status::SYNTAX_ERROR;
    }

    if (!is_error(status) && status != Parsing_status::DEPENDENCIES_NEEDED) {
        status = Parsing_status::DEPENDENCIES_NEEDED; // to avoid cyclic dependency
        for (const auto& id : identifiers) {
            id->finalize();
            ASSERT(id->status == Parsing_status::FULLY_RESOLVED);
        }
        status = Parsing_status::FULLY_RESOLVED;
    }

    return status;
}






Parsing_status read_assignment_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    if (!parent_scope->dynamic()) {
        log_error("Assignment statements are not allowd in static scopes!", it->context);
        it.current_index = it.find_matching_semicolon()+1;
        if (it.expect_failed()) return Parsing_status::FATAL_ERROR;
        else return Parsing_status::NOT_PARSED;
    }

    Owned<Abstx_assignment> o = alloc(Abstx_assignment());
    o->set_owner(parent_scope);
    o->context = it->context;
    o->start_token_index = it.current_index;

    do {
        // read variable expressions for LHS
        Owned<Variable_expression> expr = read_variable_expression(it, static_pointer_cast<Abstx_node>(o));

        if (expr == nullptr) {
            o->status = Parsing_status::SYNTAX_ERROR;
            break;
        } else if (is_error(expr->status) || (!is_error(o->status) && expr->status == Parsing_status::DEPENDENCIES_NEEDED)) {
            o->status = expr->status;
            if (is_fatal(o->status)) return o->status;
        } else if (Shared<Abstx_function_call_expression> fn_call = dynamic_pointer_cast<Abstx_function_call_expression>(expr)) {
            for (const auto& arg : fn_call->function_call->out_args) {
                ASSERT(arg);
                Owned<Variable_expression_reference> ref = alloc(Variable_expression_reference());
                ref->owner = static_pointer_cast<Abstx_node>(o);
                ref->set_reference(arg);
                ref->finalize();
                o->lhs.add(owned_static_cast<Variable_expression>(std::move(ref)));
            }
        } else {
            o->lhs.add(std::move(expr));
        }

    } while (it.eat_conditonal(Token_type::SYMBOL, ","));

    // @TODO: add support for assignment operators (+ =, - =, etc.)
    it.expect(Token_type::SYMBOL, "=");
    if (it.expect_failed()) {
        o->status = Parsing_status::SYNTAX_ERROR;
    }

    // read value expressions for RHS
    Parsing_status rhs_status = read_value_rhs(it, static_pointer_cast<Abstx_node>(o), o->rhs, false);
    if (is_error(rhs_status)) {
        o->status = rhs_status;
    }

    it.expect_end_of_statement();
    if (it.expect_failed()) {
        add_note("In assignment statement here", o->context);
        if (!is_fatal(o->status)) o->status = Parsing_status::SYNTAX_ERROR;
    }

    // perform type checking
    o->fully_parse();

    Parsing_status status = o->status;
    parent_scope->statements.add(owned_static_cast<Statement>(std::move(o)));
    return status;
}

Parsing_status Abstx_assignment::fully_parse() {
    if (is_error(status) || is_codegen_ready(status)) return status;

    // perform finalizing of expressions and type checking
    // list must either be of size 1 (all identifiers have the same value) or the same size as the number of identifiers
    if (rhs.size != 1 && rhs.size != lhs.size) {
        log_error("Wrong number of values in assignment", context);
        if (lhs.size == 1) add_note("Expected 1 value but found "+std::to_string(rhs.size), context);
        else add_note("Expected either 1 or "+std::to_string(lhs.size)+" values but found "+std::to_string(rhs.size), context);
        if (!is_error(status)) status = Parsing_status::SYNTAX_ERROR;
    } else {
        // typecheck lhs with rhs
        // everything should have types already
        size_t index = 0;
        for (const auto& id : lhs) {
            Shared<Value_expression> value_expr = rhs[index];

            // finalize expressions
            ASSERT(id); // can't be nullptr
            ASSERT(value_expr); // can't be nullptr
            id->finalize();
            value_expr->finalize();

            // check that types match
            Shared<const CB_Type> lhs_type = id->get_type();
            Shared<const CB_Type> rhs_type = value_expr->get_type();

            if (lhs_type == nullptr) {
                ASSERT(is_error(id->status) || id->status == Parsing_status::DEPENDENCIES_NEEDED);
                if (!is_error(status)) status = id->status;
            } else if (rhs_type == nullptr) {
                ASSERT(is_error(id->status) || id->status == Parsing_status::DEPENDENCIES_NEEDED);
                if (!is_error(status)) status = id->status;
            } else if (*lhs_type != *rhs_type) {
                log_error("Type of rhs doesn't match the type of lhs in assigment!", value_expr->context);
                add_note("Unable to convert type from "+rhs_type->toS()+" to "+lhs_type->toS());
                status = Parsing_status::TYPE_ERROR;
            }

            if (rhs.size != 1) ++index;
        }
    }
    if (!is_error(status) && status != Parsing_status::DEPENDENCIES_NEEDED) status = Parsing_status::FULLY_RESOLVED;
    return status;
}


/*
Parsing_status Abstx_assignment::finalize() {
    if (is_codegen_ready(status)) return status;

    if (lhs.size != rhs.size) return status;
    for (const auto& var_exp : lhs) {
        ASSERT(var_exp != nullptr)
        if (!is_codegen_ready(var_exp->finalize())) { // this expression is Owned by this statement -> finalize them too
            status = var_exp->status;
            return status;
        }
    }
    for (const auto& val_exp : rhs) {
        ASSERT(val_exp != nullptr)
        if (!is_codegen_ready(val_exp->finalize())) {
            status = val_exp->status;
            return status;
        }
    }
    // we reached the end -> we are done
    status = Parsing_status::FULLY_RESOLVED;
    return status;
}
*/







Parsing_status read_if_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {

    it.assert(Token_type::KEYWORD, "if");

    Owned<Abstx_if> o = alloc(Abstx_if()); // will be destroyed later
    o->set_owner(parent_scope);
    o->context = it->context;
    o->start_token_index = it.current_index;
    o->status = Parsing_status::NOT_PARSED;

    // read conditional scope(o)
    do {
        Owned<Abstx_if::Abstx_conditional_scope> cs = alloc(Abstx_if::Abstx_conditional_scope());
        cs->set_owner(parent_scope);
        cs->context = it->context;
        cs->start_token_index = it.current_index;

        cs->condition = read_value_expression(it, static_pointer_cast<Abstx_node>(o)); // parens not required
        if (is_error(cs->condition->status) && !is_fatal(cs->status)) cs->status = cs->condition->status;

        it.expect_current(Token_type::SYMBOL, "{");
        if (it.expect_failed() && !is_fatal(cs->status)) {
            cs->status = Parsing_status::SYNTAX_ERROR;
        } else {
            cs->scope = read_scope(it, o->parent_scope()); // braces required
            if (is_error(cs->scope->status) && !is_fatal(cs->status)) cs->status = cs->scope->status;
        }

        if (is_error(cs->status) && !is_fatal(o->status)) o->status = cs->status;
        if (is_fatal(o->status)) break;

        o->conditional_scopes.add(std::move(cs));

    } while (it.eat_conditonal(Token_type::KEYWORD, "elsif"));

    // read else scope if applicable
    if (it.eat_conditonal(Token_type::KEYWORD, "else")) {
        o->else_scope = read_scope(it, o->parent_scope());
        if (is_error(o->status) && !is_fatal(o->status)) o->status = o->else_scope->status;
    }

    if (!is_error(o->status)) {
        o->status = Parsing_status::PARTIALLY_PARSED;
    }

    LOG("Read if statement with status " << o->status << " at " << o->context.toS());

    Parsing_status status = o->status;
    parent_scope->statements.add(std::move(owned_static_cast<Statement>(std::move(o))));
    ASSERT(o == nullptr);
    return status;
}

// temporary implementations, TODO: implement these
Parsing_status read_for_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    it.assert(Token_type::KEYWORD, "for");

    Owned<Abstx_for> o = alloc(Abstx_for());
    o->set_owner(parent_scope);
    o->context = it->context;
    o->start_token_index = it.current_index;

    // it.expect(Token_type::SYMBOL, "(");

    log_error("For statement not yet implemented!", o->context);
    o->status = Parsing_status::SYNTAX_ERROR;
    it.eat_token(); // eat token for now to ensure we don't get stuck in an infinite loop

    Parsing_status status = o->status;
    parent_scope->statements.add(std::move(owned_static_cast<Statement>(std::move(o))));
    ASSERT(o == nullptr);
    return status;
}

Parsing_status read_while_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    it.assert(Token_type::KEYWORD, "while");

    Owned<Abstx_while> o = alloc(Abstx_while());
    o->set_owner(parent_scope);
    o->context = it->context;
    o->start_token_index = it.current_index;

    // it.expect(Token_type::SYMBOL, "(");

    log_error("While statement not yet implemented!", o->context);
    o->status = Parsing_status::SYNTAX_ERROR;
    it.eat_token(); // eat token for now to ensure we don't get stuck in an infinite loop

    Parsing_status status = o->status;
    parent_scope->statements.add(std::move(owned_static_cast<Statement>(std::move(o))));
    ASSERT(o == nullptr);
    return status;
}

Parsing_status read_return_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    it.assert(Token_type::KEYWORD, "return");

    Owned<Abstx_return> o = alloc(Abstx_return());
    o->set_owner(parent_scope);
    o->context = it->context;
    o->start_token_index = it.current_index;

    // it.expect(Token_type::SYMBOL, "(");

    log_error("Return statement not yet implemented!", o->context);
    o->status = Parsing_status::SYNTAX_ERROR;
    it.eat_token(); // eat token for now to ensure we don't get stuck in an infinite loop

    Parsing_status status = o->status;
    parent_scope->statements.add(std::move(owned_static_cast<Statement>(std::move(o))));
    ASSERT(o == nullptr);
    return status;
}

Parsing_status read_defer_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    it.assert(Token_type::KEYWORD, "defer");

    Owned<Abstx_defer> o = alloc(Abstx_defer());
    o->set_owner(parent_scope);
    o->context = it->context;
    o->start_token_index = it.current_index;

    // it.expect(Token_type::SYMBOL, "(");

    log_error("Defer statement not yet implemented!", o->context);
    o->status = Parsing_status::SYNTAX_ERROR;
    it.eat_token(); // eat token for now to ensure we don't get stuck in an infinite loop

    Parsing_status status = o->status;
    parent_scope->statements.add(std::move(owned_static_cast<Statement>(std::move(o))));
    ASSERT(o == nullptr);
    return status;
}

Parsing_status read_using_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    it.assert(Token_type::KEYWORD, "using");

    Owned<Abstx_using> o = alloc(Abstx_using());
    o->set_owner(parent_scope);
    o->context = it->context;
    o->start_token_index = it.current_index;

    // for now: just read to matching semicolon
    it.current_index = it.find_matching_semicolon() + 1;
    if (it.expect_failed()) {
        o->status = Parsing_status::SYNTAX_ERROR;
    }

    if (!is_error(o->status)) o->status = Parsing_status::PARTIALLY_PARSED;

    LOG("read using statement with status " << o->status << "; next token: " << it->token);

    Parsing_status status = o->status;
    parent_scope->using_statements.add(o);
    parent_scope->statements.add(std::move(owned_static_cast<Statement>(std::move(o))));
    ASSERT(o == nullptr);
    return status;
}

Parsing_status read_c_code_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    // @TODO: allow generic constant value expressions of type string
    // for now, only accept string literals
    // it is so easy that everything can be done here

    Owned<Abstx_c_code> o = alloc(Abstx_c_code());
    o->set_owner(parent_scope);
    o->context = it->context;
    o->start_token_index = it.current_index;

    it.assert(Token_type::COMPILER_COMMAND, "#c");
    o->c_code = it.expect(Token_type::STRING).token;
    if (it.expect_failed()) {
        o->status = Parsing_status::SYNTAX_ERROR;
    }

    it.expect_end_of_statement();
    if (it.expect_failed()) {
        add_note("In #C statement here", o->context);
        o->status = Parsing_status::FATAL_ERROR;
    }

    if (!is_error(o->status)) o->status = Parsing_status::FULLY_RESOLVED;
    Parsing_status status = o->status;
    parent_scope->statements.add(owned_static_cast<Statement>(std::move(o)));
    return status;
}

Parsing_status Abstx_c_code::fully_parse() {
    // @TODO: allow generic constant value expressions of type string
    // for now, only string literals are allowed, and all parsing is done in read_c_code_statement()
    ASSERT(is_error(status) || is_codegen_ready(status));
    return status;

    // assert first token is #C
    // read value expression
    // if next token is not ';' error
    // if expression.get_type() is not string, error
    // if expression.has_constant_value is not true, error
    // grab the value through expression.get_constant_value
}





Parsing_status read_value_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    Owned<Value_expression> expr = read_value_expression(it, static_pointer_cast<Abstx_node>(parent_scope));
    Shared<Abstx_function_call_expression> fc = dynamic_pointer_cast<Abstx_function_call_expression>(expr);
    if (expr != nullptr && fc == nullptr) {
        log_warning("Non-function call used as a statement - it might be ignored", expr->context);
    }
    it.expect_end_of_statement();
    if (it.expect_failed()) {
        ASSERT(expr);
        add_note("in value statement here", expr->context); // @debug
        return Parsing_status::SYNTAX_ERROR;
    }
    else return expr->status;
    // expr will be deallocated; but if any function calls were included in the expression, they will have been inserted in the
    //   parent scope as individual function call statements (and won't be deallocated)
}


// maybe should this also return only Parsing_status?
Shared<Abstx_function_call> read_run_expression(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    ASSERT(false, "NYI");
    return nullptr;
}





Parsing_status Abstx_scope::fully_parse() {
    // if global scope: everything is read and has a type
    // if dynamic scope: everything has to be fully parsed in order
    // in either case, everything should be able to be fully parsed immediately

    if (is_error(status) || is_codegen_ready(status)) return status;

    auto it = parse_begin();

    ASSERT(!dynamic() || it.compare(Token_type::SYMBOL, "{"));
    ASSERT(statements.size == 0, "scope has " << statements.size << " statements");

    // read scope statements; this adds the statements properly
    // if this is a dynamic scope, it also finalizes everything properly
    read_scope_statements(it, this); // also sets status

    LOG("fully parsed scope with status " << status << " with " << statements.size << " statements at " << context.toS());
    return status;
}







Parsing_status Abstx_if::fully_parse() {
    LOG("Abstx_if::fully_parse");

    if (is_codegen_ready(status) || is_error(status)) return status;
    for (const auto& cs : conditional_scopes) {
        if (!is_codegen_ready(cs->fully_parse())) {
            status = cs->status;
            LOG("error: " << status);
            return status;
        }
    }
    if (else_scope != nullptr && !is_codegen_ready(else_scope->fully_parse())) {
        status = else_scope->status;
        LOG("error: " << status);
        return status;
    }
    // if (then_scope != nullptr && !is_codegen_ready(then_scope->fully_parse())) {
    //     status = then_scope->status;
    //     return status;
    // }
    // we reached the end -> we are done
    status = Parsing_status::FULLY_RESOLVED;
    return status;
}


Parsing_status Abstx_using::fully_parse() {
    if (is_codegen_ready(status)) return status;

    auto it = parse_begin();
    subject = read_value_expression(it, this);
    subject->finalize();

    status = subject->status;
    return status;
}

Parsing_status Abstx_defer::fully_parse() {
    if (is_codegen_ready(status)) return status;
    // @TODO implement
    // we reached the end -> we are done
    // status = Parsing_status::FULLY_RESOLVED;
    return status;
}

Parsing_status Abstx_return::fully_parse() {
    if (is_codegen_ready(status)) return status;
    // @TODO implement
    // we reached the end -> we are done
    // status = Parsing_status::FULLY_RESOLVED;
    return status;
}

Parsing_status Abstx_while::fully_parse() {
    if (is_codegen_ready(status)) return status;
    // @TODO implement
    // we reached the end -> we are done
    // status = Parsing_status::FULLY_RESOLVED;
    return status;
}

Parsing_status Abstx_for::fully_parse() {
    if (is_codegen_ready(status)) return status;
    // @TODO implement
    // we reached the end -> we are done
    // status = Parsing_status::FULLY_RESOLVED;
    return status;
}



/*

Parsing_status Abstx_declaration::finalize() override {
    if (is_codegen_ready(status)) return status;

    if (rhs.size != 0 && identifiers.size != rhs.size) {
        status = Parsing_status::TYPE_ERROR;
        return status;
    }
    for (const auto& id : identifiers) {
        ASSERT(id != nullptr)
        if (!is_codegen_ready(id->finalize())) {
            status = id->status; // @todo: save all dependencies in a list for later (maybe)
            return status;
        }
    }
    for (const auto& val_exp : rhs) {
        ASSERT(val_exp != nullptr)
        if (!is_codegen_ready(val_exp->finalize())) {
            status = val_exp->status;
            return status;
        }
    }
    // we reached the end -> we are done
    status = Parsing_status::FULLY_RESOLVED;
    return status;
}


Parsing_status Abstx_for::finalize() override {
    if (is_codegen_ready(status)) return status;

    // range
    ASSERT(range != nullptr);
    if (anonymous_range) {
        range->name = "_range_" + std::to_string(get_unique_id());
    }
    if (!is_codegen_ready(range->finalize())) {
        status = range->status;
        return status;
    }

    // scope
    ASSERT(scope != nullptr);
    if (!is_codegen_ready(scope->finalize())) {
        status = scope->status;
        return status;
    }

    // it
    ASSERT(it != nullptr);
    if (!is_codegen_ready(it->status)) {
        if (is_error(it->status)) status = it->status;
        else status = Parsing_status::DEPENDENCIES_NEEDED;
        return status;
    }

    status = Parsing_status::FULLY_RESOLVED;
    return status;
}


Parsing_status Abstx_function_call::fully_parse() override {
    if (status == Parsing_status::FULLY_RESOLVED || is_codegen_ready(status)) return status;
    ASSERT(function_pointer != nullptr);
    if (!is_codegen_ready(function_pointer->fully_parse())) {
        status = function_pointer->status;
        return status;
    }
    return status;
}
Parsing_status Abstx_function_call::finalize() override {
    if (is_codegen_ready(status)) return status;

    ASSERT(function_pointer != nullptr);
    if (!is_codegen_ready(function_pointer->finalize())) {
        status = function_pointer->status;
        return status;
    }

    Shared<Abstx_identifier_reference> fn_id = dynamic_pointer_cast<Abstx_identifier_reference>(function_pointer);
    if (fn_id && fn_id->id) {
        function = dynamic_pointer_cast<Abstx_function>(fn_id->id->value_expression);
    }

    if (function != nullptr) {
        if (!is_codegen_ready(function->status)) {
            if (is_error(function->status)) status = function->status;
            else status = Parsing_status::DEPENDENCIES_NEEDED;
            return status;
        }
    }

    Shared<const CB_Function> fn_type = dynamic_pointer_cast<const CB_Function>(function_pointer->get_type());
    ASSERT(fn_type != nullptr);

    // check types
    if (in_args.size != fn_type->in_types.size) {
        // @todo generate compile error
        status = Parsing_status::TYPE_ERROR;
        return status;
    }
    for (int i = 0; i < in_args.size; ++i) {
        if (in_args[i]->get_type() != fn_type->in_types[i]) {
            // @todo generate compile error
            status = Parsing_status::TYPE_ERROR;
            return status;
        }
    }

    if (out_args.size != fn_type->out_types.size) {
        // @todo generate compile error
        status = Parsing_status::TYPE_ERROR;
        return status;
    }
    for (int i = 0; i < out_args.size; ++i) {
        if (out_args[i]->get_type() != fn_type->out_types[i]) {
            // @todo generate compile error
            status = Parsing_status::TYPE_ERROR;
            return status;
        }
    }

    // we reached the end -> we are done
    status = Parsing_status::FULLY_RESOLVED;
    return status;
}


Parsing_status Abstx_if::Abstx_conditional_scope::finalize() override {
    if (is_codegen_ready(status)) return status;

    if (!is_codegen_ready(condition->finalize())) {
        status = condition->status;
        return status;
    }
    Shared<const CB_Type> type = condition->get_type();
    if (*type != *CB_Bool::type) {
        status = Parsing_status::TYPE_ERROR;
        return status;
    }
    if (!is_codegen_ready(scope->finalize())) {
        status = scope->status;
        return status;
    }
    // we reached the end -> we are done
    status = Parsing_status::FULLY_RESOLVED;
    return status;
}



Parsing_status Abstx_while::finalize() override {
    if (is_codegen_ready(status)) return status;

    if (!is_codegen_ready(condition->finalize())) {
        status = Parsing_status::DEPENDENCIES_NEEDED;
        return status;
    }
    Shared<const CB_Type> type = condition->get_type();
    if (*type != *CB_Bool::type) {
        status = Parsing_status::TYPE_ERROR;
        return status;
    }
    if (!is_codegen_ready(scope->finalize())) {
        status = Parsing_status::DEPENDENCIES_NEEDED;
        return status;
    }
    // we reached the end -> we are done
    status = Parsing_status::FULLY_RESOLVED;
    return status;
}
*/




