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
    } else {
        // read scope
        s->scope = read_scope(it, parent_scope);
        ASSERT(s->scope != nullptr);
        // s->scope->set_owner(s); // no need to update owner; parent_scope is good enough

        s->status = s->scope->status;
        parent_scope->statements.add(std::move(owned_static_cast<Statement>(std::move(o))));
        ASSERT(o == nullptr);
    }
    ASSERT(s != nullptr);
    return s->status;
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

    bool error = false;
    while (1) {
        // allocate abstx identifier and set base info
        Owned<Abstx_identifier> id = alloc(Abstx_identifier());
        id->set_owner(s);
        s->context = it->context;
        s->start_token_index = it.current_index;

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
            // no specific error in the declaration, just ignore the identifier
        } else {
            parent_scope->identifiers[id->name] = (Shared<Abstx_identifier>)id;
            s->identifiers.add(std::move(id));
        }

        if (it.compare(Token_type::SYMBOL, ",")) continue; // one more

        it.expect(Token_type::SYMBOL, ":");
        if (it.expect_failed()) {
            s->status = Parsing_status::SYNTAX_ERROR;
            break;
        }
    }

    if (is_error(s->status)) {
        add_note("In declaration statement here", s->context);
        s->status = Parsing_status::SYNTAX_ERROR;
    }

    // we are done reading -> set the start_token_index to the first value of RHS
    s->start_token_index = it.current_index;

    // find the closing ';'
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
        s->finalize();
        // @todo: maybe log error if not successful? (that might be done inside fully_parse or finalize, though)
    }

    // add it to the scope
    parent_scope->statements.add(std::move(owned_static_cast<Statement>(std::move(o))));

    return s->status;
}












// temporary implementations, TODO: implement these
Parsing_status read_if_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) { return Parsing_status::NOT_PARSED; }
Parsing_status read_for_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) { return Parsing_status::NOT_PARSED; }
Parsing_status read_while_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) { return Parsing_status::NOT_PARSED; }
Parsing_status read_return_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) { return Parsing_status::NOT_PARSED; }
Parsing_status read_defer_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) { return Parsing_status::NOT_PARSED; }
Parsing_status read_using_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) { return Parsing_status::NOT_PARSED; }
Parsing_status read_c_code_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) { return Parsing_status::NOT_PARSED; }
Parsing_status read_assignment_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) { return Parsing_status::NOT_PARSED; }
Parsing_status read_value_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) { return Parsing_status::NOT_PARSED; }

// maybe should this also return only Parsing_status?
Shared<Abstx_function_call> read_run_expression(Token_iterator& it, Shared<Abstx_scope> parent_scope) { return nullptr; }

