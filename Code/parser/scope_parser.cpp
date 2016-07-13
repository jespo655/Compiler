#include "parser.h"



// read_static_scope: read and partially parse statements until end of scope
//      also store all using-statements in a separate list for easier access
//      For each run-statement, find global scope and put it there for later
// The iterator should be at the opening '{'.
// If global scope, where there is no '{}', it should be at start of the first statement.
// FIXME: add a way to reach all #run-statements for pass 2
std::shared_ptr<Scope> read_static_scope(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(parent_scope == nullptr || parent_scope->dynamic = false);

    auto scope = std::shared_ptr<Scope>(new Scope());
    scope->start_token_index = it.current_index;
    scope->context = it->context;
    scope->owner = parent_scope;
    scope->dynamic = false;

    bool global = true;
    if (it->type == Token_type::SYMBOL && it->token == "{") {
        global == false;
        it.eat_token();
    }
    // if global, read to eof
    // else, read to '}'

    while (!it->is_eof() && !(it->type == Token_type::SYMBOL && it->token == "}")) {

        std::shared_ptr<Statement> s = read_statement(it, scope);
        ASSERT(s != nullptr);
        if (s->status == Parsing_status::FATAL_ERROR) {
            scope->status = Parsing_status::FATAL_ERROR;
            return Parsing_status::FATAL_ERROR;
        }

        scope->statements.push_back(s);
    }
    ASSERT(it->is_eof() || (it->type == Token_type::SYMBOL && it->token == "}"));

    if (it->type == Token_type::SYMBOL && it->token == "}") {
        if (global) {
            log_error("Unexpected token "+it->token+" in unresolved statement.", it->context);
            scope->status = Parsing_status::FATAL_ERROR;
            return Parsing_status::FATAL_ERROR;
        } else {
            it.eat_token(); // eat the '}' token
        }
    } else {
        ASSERT(it->is_eof());
        if(!global) {
            log_error("Missing '}' at end of scope: found unexpected end of file", it->context);
            add_note("In scope that started here:", scope->context);
            scope->status = Parsing_status::FATAL_ERROR;
            return Parsing_status::FATAL_ERROR;
        }
    }

    ASSERT(scope->status != Parsing_status::FATAL_ERROR); // in this case we should have returned already.
    scope->status = Parsing_status::PARTIALLY_PARSED;
    return scope;
}





// All dynamic scopes will be read (and skipped) as a part of statements that are
// part of static scopes (e.g. the body of a function).
// The dynamic scope are first actually read in the middle of a compile time evaluation
// (either in a #run or when making the output bytecode), so each statement it can and
// should be fully parsed immediately when read.
std::shared_ptr<Scope> read_dynamic_scope(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(it->type == Token_type::SYMBOL && t->token == "{");

    auto scope = std::shared_ptr<Scope>(new Scope());
    scope->start_token_index = it.current_index;
    scope->context = it->context;
    scope->owner = parent_scope;
    scope->dynamic = true;

    while (!it->is_eof() && !(it->type == Token_type::SYMBOL && it->token == "}")) {

        std::shared_ptr<Statement> s = read_statement(it, scope);
        ASSERT(s != nullptr);

        fully_resolve_statement(s);
        if (is_error(s->status)) {
            scope->status = s->status;
            return scope->status;
        }

        ASSERT(s->status == Parsing_status::FULLY_PARSED);
        scope->statements.push_back(s);
    }
    ASSERT(it->type == Token_type::SYMBOL && it->token == "}");
    it.eat_token(); // eat the '}' token

    scope->status = Parsing_status::FULLY_PARSED;

    return scope;
}







