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
        // create scope
        s->scope = alloc(Abstx_scope(parent_scope->flags));
        s->scope->set_owner(s);
        s->scope->context = it->context;
        s->scope->start_token_index = it.current_index;

        // parse and resolve all statements in the scope
        Parsing_status status = Parsing_status::NOT_PARSED;
        while (!is_error(status) && !it.compare(Token_type::SYMBOL, "}")) {
            status = read_statement(it, s->scope);
            // fatal error -> give up
            // other error -> failed to parse dynamic scope, but we can continue with other stuff if we find the closing brace
            if (is_error(status) && !is_fatal(status)) {
                it.current_index = it.find_matching_brace(s->start_token_index);
                if (it.expect_failed()) {
                    s->scope->status = Parsing_status::FATAL_ERROR;
                } else {
                    s->scope->status = status;
                }
                break;
            }
            if (is_eof(it->type)) {
                log_error("Unexpected end of file in the middle of a scope", it->context);
                add_note("In anonymous scope that started here", s->context);
                s->scope->status = Parsing_status::FATAL_ERROR;
                break;
            }
        }
        if (!is_fatal(s->scope->status)) {
            it.assert(Token_type::SYMBOL, "}"); // we should have found this already. Now eat it so we return with it pointing to after the brace
        }
        s->status = s->scope->status;
        parent_scope->statements.add(std::move(owned_static_cast<Statement>(std::move(o))));
        ASSERT(o == nullptr);
    }
    ASSERT(s != nullptr);
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
Parsing_status read_declaration_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) { return Parsing_status::NOT_PARSED; }
Parsing_status read_assignment_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) { return Parsing_status::NOT_PARSED; }
Parsing_status read_value_statement(Token_iterator& it, Shared<Abstx_scope> parent_scope) { return Parsing_status::NOT_PARSED; }

// maybe should this also return only Parsing_status?
Shared<Abstx_function_call> read_run_expression(Token_iterator& it, Shared<Abstx_scope> parent_scope) { return nullptr; }
