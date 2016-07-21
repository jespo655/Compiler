#include "parser.h"






std::shared_ptr<Global_scope> read_global_scope(const std::vector<Token>& tokens, const std::string& name)
{
    auto global_scope = std::shared_ptr<Global_scope>(new Global_scope(tokens));
    global_scope->file_name = name;

    Token_iterator it = global_scope->iterator();
    global_scope->start_token_index = 0;
    global_scope->context = it->context;

    global_scope->dynamic = false;

    while (!it->is_eof()) {

        std::shared_ptr<Statement> s = read_statement(it, global_scope);
        ASSERT(s != nullptr);
        if (s->status == Parsing_status::FATAL_ERROR) {
            global_scope->status = Parsing_status::FATAL_ERROR;
            return global_scope;
        }

        global_scope->statements.push_back(s);
    }

    global_scope->status = Parsing_status::PARTIALLY_PARSED;
    return global_scope;

}







// read_static_scope: read and partially parse statements until end of scope
//      also store all using-statements in a separate list for easier access
//      For each run-statement, find global scope and put it there for later
// The iterator should be at the opening '{'.
// If global scope, where there is no '{}', it should be at start of the first statement.
// FIXME: add a way to reach all #run-statements for pass 2
std::shared_ptr<Scope> read_static_scope(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(parent_scope != nullptr && parent_scope->dynamic == false);

    auto scope = std::shared_ptr<Scope>(new Scope());
    scope->start_token_index = it.current_index;
    scope->context = it->context;
    scope->owner = parent_scope;
    scope->dynamic = false;

    while (!it->is_eof() && !(it->type == Token_type::SYMBOL && it->token == "}")) {

        std::shared_ptr<Statement> s = read_statement(it, scope);
        ASSERT(s != nullptr);
        if (s->status == Parsing_status::FATAL_ERROR) {
            scope->status = Parsing_status::FATAL_ERROR;
            return scope;
        }

        scope->statements.push_back(s);
    }

    if (it->is_eof()) {
        log_error("Missing '}' at end of scope: found unexpected end of file", it->context);
        add_note("In scope that started here:", scope->context);
        scope->status = Parsing_status::FATAL_ERROR;
        return scope;
    }

    ASSERT (it->type == Token_type::SYMBOL && it->token == "}");
    it.eat_token(); // eat the '}' token

    ASSERT(scope->status != Parsing_status::FATAL_ERROR); // in this case we should have returned already.
    scope->status = Parsing_status::PARTIALLY_PARSED;
    return scope;
}





// All dynamic scopes will be read (and skipped) as a part of statements that are
// part of static scopes (e.g. the body of a function).
// The dynamic scope are first actually read in the middle of a compile time evaluation
// (either in a #run or when making the output bytecode), so each statement it can and
// should be fully parsed immediately when read.

// FIXME: remove token assertions and log_error instead, we can't assume
// that we already did the global parsing pass before
std::shared_ptr<Scope> compile_dynamic_scope(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(it->type == Token_type::SYMBOL && it->token == "{");

    auto scope = std::shared_ptr<Scope>(new Scope());
    scope->start_token_index = it.current_index;
    scope->context = it->context;
    scope->owner = parent_scope;
    scope->dynamic = true;

    while (!it->is_eof() && !(it->type == Token_type::SYMBOL && it->token == "}")) {

        std::shared_ptr<Statement> s = compile_statement(it, scope);
        ASSERT(s != nullptr);

        if (s->status == Parsing_status::FATAL_ERROR) {
            scope->status = s->status;
            return scope;
        }
        else if (is_error(s->status)) {
            scope->status = s->status;
            it.current_index = it.find_matching_brace();
            break;
        }

        ASSERT(s->status == Parsing_status::FULLY_RESOLVED);
        scope->statements.push_back(s);
    }
    ASSERT(it->type == Token_type::SYMBOL && it->token == "}");
    it.eat_token(); // eat the '}' token

    if (!is_error(scope->status))
        scope->status = Parsing_status::FULLY_RESOLVED;
    return scope;
}







