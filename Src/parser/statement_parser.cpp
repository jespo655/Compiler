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

    if (it->is_eof() || (it->type == Token_type::SYMBOL && it->token == "}")) {
        // end of scope -> no error, just return
        return Parsing_status::NOT_PARSED;

    } else if (it->type == Token_type::SYMBOL && it->token == ";") {
        // empty statement, give warning and ignore
        log_warning("Additional ';' found", it->context);
        it.eat_token(); // eat the ';' token
        return read_statement(it, parent_scope);

    } else if (it->type == Token_type::SYMBOL && it->token == "{") {
        // nested scope
        return read_anonymous_scope(it, parent_scope);

    } else if (it->type == Token_type::KEYWORD && it->token == "if") {
        // if statement
        return read_if_statement(it, parent_scope);

    } else if (it->type == Token_type::KEYWORD && it->token == "for") {
        // for statement
        return read_for_statement(it, parent_scope);

    } else if (it->type == Token_type::KEYWORD && it->token == "while") {
        // while statement
        return read_while_statement(it, parent_scope);

    } else if (it->type == Token_type::KEYWORD && it->token == "return") {
        // return statement
        return read_return_statement(it, parent_scope);

    } else if (it->type == Token_type::KEYWORD && it->token == "defer") {
        // defer statement
        return read_defer_statement(it, parent_scope);

    } else if (it->type == Token_type::KEYWORD && it->token == "using") {
        // using statement
        return read_using_statement(it, parent_scope);

    } else if (it->type == Token_type::COMPILER_COMMAND && it->token == "#c") {
        // c code statement
        return read_c_code_statement(it, parent_scope);

    } else if (it->type == Token_type::COMPILER_COMMAND && it->token == "#run") {
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
                    add_note("In unresolved statement that started here: ", it.look_at(start_index).context);
                    return Parsing_status::FATAL_ERROR;
                }
            }
        }
    }

    ASSERT(false); // should never be reached
    return Parsing_status::NOT_PARSED;
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

    // parse and resolve all statements in the scope
    Parsing_status status = Parsing_status::NOT_PARSED;
    while (!is_error(status) && !it.compare(Token_type::SYMBOL, "}")) {
        status = read_statement(it, scope);
        // fatal error -> give up
        // other error -> failed to parse dynamic scope, but we can continue with other stuff if we find the closing brace
        if (is_error(status) && !is_fatal(status)) {
            // it.current_index = it.find_matching_brace(scope->start_token_index); // gives better error messages
            it.current_index = it.find_matching_brace(); // faster
            if (it.expect_failed()) {
                add_note("In scope that started here", scope->context); // "good enough" error message for fast solution
                scope->status = Parsing_status::FATAL_ERROR;
            } else {
                scope->status = status;
            }
            break;
        }
        if (is_eof(it->type)) {
            log_error("Unexpected end of file in the middle of a scope", it->context);
            add_note("In scope that started here", scope->context);
            scope->status = Parsing_status::FATAL_ERROR;
            break;
        }
    }
    if (!is_fatal(scope->status)) {
        it.assert(Token_type::SYMBOL, "}"); // we should have found this already. Now eat it so we return with it pointing to after the brace
    }
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
        log_error("Anonymous scope used in static context. That makes no sense! Did you mean \"using {...};\"?", it->context);
        s->status = Parsing_status::SYNTAX_ERROR;
        it.current_index = it.find_matching_brace();

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

Parsing_status Abstx_anonymous_scope::fully_parse() {
    if (status != Parsing_status::PARTIALLY_PARSED && status != Parsing_status::DEPENDENCIES_NEEDED) return status;
    // @TODO
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

    Owned<Abstx_declaration> o = alloc(Abstx_declaration()); // will be destroyed later
    Shared<Abstx_declaration> s = o; // shared pointer that we can use and modify however we like
    s->set_owner(parent_scope);
    s->context = it->context;
    s->start_token_index = it.current_index;

    // TODO: add special check for if the first token is ':'

    // std::cout << "reading declaration statement" << std::endl; // @debug
    while (1) {
        // allocate abstx identifier and set base info
        Owned<Abstx_identifier> id = alloc(Abstx_identifier());
        id->set_owner(s);
        id->context = it->context;
        id->start_token_index = it.current_index;

        // std::cout << "reading declared identifier starting with token " << it->toS() << " at index " << it.current_index << std::endl; // @debug

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

        if (it.compare(Token_type::SYMBOL, ",")) {
            it.eat_token(); // eat token and continue
        } else {
            break; // go to next step
        }
    }

    s->start_token_index = it.current_index; // update start_token_index to point to the ':' token
    it.expect(Token_type::SYMBOL, ":");
    if (it.expect_failed()) {
        s->status = Parsing_status::SYNTAX_ERROR;
    }

    if (is_error(s->status)) {
        add_note("In declaration statement here", s->context);
    }

    // std::cout << "done reading declaration" << std::endl; // @debug

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

    // std::cout << "fully parsing declaration statement starting with token " << it->toS() << " at index " << it.current_index << std::endl; // @debug

    // @todo: allow constant function calls as type identifiers
    // @todo: allow #run function calls as type identifiers
    // @todo: don't overwrite FATAL_ERROR with different error message
    // @todo: allow function calls as RHS

    if (!it.compare(Token_type::SYMBOL, "=") && !it.compare(Token_type::SYMBOL, ":")) {
        // read list of types
        auto p_scope = parent_scope();
        while(1) {

            Owned<Value_expression> type_expr = read_value_expression(it, p_scope);
            if (type_expr == nullptr) {
                status = Parsing_status::SYNTAX_ERROR; // unable to read value expression
                return status; // give up
            }
            type_expr->owner = this;
            if (is_error(type_expr->status)) {
                add_note("In declaration statement here", context);
                status = type_expr->status;
                return status;

            } else if (type_expr->status == Parsing_status::DEPENDENCIES_NEEDED) {
                std::cout << "type expression has Parsing_status::DEPENDENCIES_NEEDED" << std::endl;
                status = type_expr->status;
                // continue reading
            } else {
                Shared<const CB_Type> t = type_expr->get_type();
                ASSERT(t != nullptr, "Value expression should have error status if it can't infer type after read_value_expression()");
                if (*t != *CB_Type::type) {
                    log_error("Non-type expresson used as type", type_expr->context);
                    add_note("In declaration statement here", context);
                    status = Parsing_status::TYPE_ERROR;
                } else if(!type_expr->has_constant_value()) {
                    log_error("Unable to infer type from type expression at compile time", type_expr->context);
                    add_note("In declaration statement here", context);
                    add_note("All types must be known at compile time!");
                    status = Parsing_status::TYPE_ERROR;
                }
            }

            type_expressions.add(std::move(type_expr));

            if (it.compare(Token_type::SYMBOL, ",")) {
                it.eat_token(); // eat token and continue
            } else {
                break; // go to next step
            }
        }

        // std::cout << "assigning types" << std::endl; // @debug

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

        // std::cout << "reading " << (constant?"constant":"non-constant") << " values in declaration" << std::endl; // @debug

        // read list of values
        auto p_scope = parent_scope();
        while(1) {

            // std::cout << "reading " << (constant?"constant":"non-constant") << " value" << std::endl; // @debug

            Owned<Value_expression> value_expr = read_value_expression(it, p_scope);
            if (value_expr == nullptr) {
                status = Parsing_status::SYNTAX_ERROR; // unable to read value expression
                return status; // give up
            }
            value_expr->owner = this;
            if (is_error(value_expr->status)) {
                add_note("In declaration statement here", context);
                status = value_expr->status;
                return status;

            } else if (value_expr->status == Parsing_status::DEPENDENCIES_NEEDED) {
                status = value_expr->status;
                // continue reading
            } else {
                ASSERT(value_expr->get_type() != nullptr, "Value expression should have error status if it can't infer type after read_value_expression()");
                if(constant && !value_expr->has_constant_value()) {
                    log_error("Unable to declare a constant value from a non-constant value expression", value_expr->context);
                    add_note("In declaration statement here", context);
                    value_expr->status = Parsing_status::COMPILE_TIME_ERROR;
                    status = Parsing_status::COMPILE_TIME_ERROR;
                }
            }

            value_expressions.add(std::move(value_expr));

            if (it.compare(Token_type::SYMBOL, ",")) {
                it.eat_token(); // eat token and continue
            } else {
                break; // go to next step
            }
        }

        // std::cout << "assigning " << (constant?"constant":"non-constant") << " values" << std::endl; // @debug

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
                id->value_expression = value_expr;

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

    // std::cout << "end of declaration statement" << std::endl; // @debug

    it.expect_end_of_statement();
    if (it.expect_failed()) {
        add_note("In declaration statement here", context);
        status = Parsing_status::SYNTAX_ERROR;
    }

    if (!is_error(status) && status != Parsing_status::DEPENDENCIES_NEEDED) {
        for (const auto& id : identifiers) {
            id->finalize();
            ASSERT(id->status == Parsing_status::FULLY_RESOLVED);
        }
        status = Parsing_status::FULLY_RESOLVED;
    }

    return status;
}











Parsing_status read_assignment_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {

    Owned<Abstx_declaration> o = alloc(Abstx_declaration()); // will be destroyed later
    Shared<Abstx_declaration> s = o; // shared pointer that we can use and modify however we like
    s->set_owner(parent_scope);
    s->context = it->context;
    s->start_token_index = it.current_index;

    ASSERT(false, "NYI");
    return Parsing_status::NOT_PARSED;
}









// temporary implementations, TODO: implement these
Parsing_status read_if_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    ASSERT(false, "NYI"); return Parsing_status::NOT_PARSED;
}

Parsing_status read_for_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    ASSERT(false, "NYI"); return Parsing_status::NOT_PARSED;
}

Parsing_status read_while_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    ASSERT(false, "NYI"); return Parsing_status::NOT_PARSED;
}

Parsing_status read_return_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    ASSERT(false, "NYI"); return Parsing_status::NOT_PARSED;
}

Parsing_status read_defer_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    ASSERT(false, "NYI"); return Parsing_status::NOT_PARSED;
}

Parsing_status read_using_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    ASSERT(false, "NYI"); return Parsing_status::NOT_PARSED;
}

Parsing_status read_c_code_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    ASSERT(false, "NYI"); return Parsing_status::NOT_PARSED;
}

Parsing_status read_value_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) {
    ASSERT(false, "NYI"); return Parsing_status::NOT_PARSED;
}

Parsing_status Abstx_function_call::fully_parse() {
    ASSERT(false, "NYI"); return Parsing_status::NOT_PARSED;
}


// maybe should this also return only Parsing_status?
Shared<Abstx_function_call> read_run_expression(Token_iterator& it, Shared<Abstx_scope> parent_scope) { return nullptr; }






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
    return status;function_pointer
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



Parsing_status Abstx_if::finalize() override {
    if (is_codegen_ready(status)) return status;
    for (const auto& cs : conditional_scopes) {
        if (!is_codegen_ready(cs->finalize())) {
            status = cs->status;
            return status;
        }
    }
    if (else_scope != nullptr && !is_codegen_ready(else_scope->finalize())) {
        status = else_scope->status;
        return status;
    }
    // if (then_scope != nullptr && !is_codegen_ready(then_scope->finalize())) {
    //     status = then_scope->status;
    //     return status;
    // }
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


Parsing_status Abstx_c_code::fully_parse() {
    // assert first token is #C
    // read value expression
    // if next token is not ';' error
    // if expression.get_type() is not string, error
    // if expression.has_constant_value is not true, error
    // grab the value through expression.get_constant_value
}




