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
        return read_anonymous_static_scope(it, parent_scope);

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


// temporary implementations implement these
Parsing_status read_anonymous_static_scope(Token_iterator& it, Shared<Abstx_scope> parent_scope) { return Parsing_status::NOT_PARSED; }
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
