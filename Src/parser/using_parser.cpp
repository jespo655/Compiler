#include "parser.h"
#include "../abstx/using.h"
#include "../abstx/str.h"
#include "../compile_time/compile_time.h"

// syntax:
// using val_expr;
std::shared_ptr<Using_statement> read_using_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(it->type == Token_type::KEYWORD && it->token == "using");
    auto us = std::shared_ptr<Using_statement>(new Using_statement());
    us->owner = parent_scope;
    us->context = it->context;
    us->start_token_index = it.current_index;

    it.current_index = it.find_matching_semicolon() + 1;

    if (it.error) us->status = Parsing_status::FATAL_ERROR;
    else us->status = Parsing_status::PARTIALLY_PARSED;

    return us;
}



Parsing_status fully_resolve_using(std::shared_ptr<Using_statement> us)
{
    ASSERT(us != nullptr);
    if (us->status == Parsing_status::FULLY_RESOLVED || is_error(us->status))
        return us->status;

    auto parent_scope = us->parent_scope();
    ASSERT(parent_scope != nullptr);

    // ASSERT(false, "using NYI"); // beware of infinite dependency loops
    us->status = Parsing_status::DEPENDENCIES_NEEDED;

    Token_iterator it = get_global_scope(us->parent_scope())->iterator(us->start_token_index+1);
    // Token_iterator it = get_iterator(us, us->start_token_index+1); // FIXME: undefined reference to get_iterator(...) ??????

    us->subject = compile_value_expression(it, parent_scope);
    us->status = us->subject->status;

    if (is_error(us->status) || us->status == Parsing_status::DEPENDENCIES_NEEDED) {
        return us->status;
    }

    // Evaluate the expression and get the scope from it
    Value value = eval(us->subject); // compile time execution, yay!

    std::shared_ptr<Scope> scope{nullptr};
    auto type = value.get_type();
    if (auto t = std::dynamic_pointer_cast<Type_scope>(type)) {
        // direct scope include
        scope = t->cpp_value(value.get_value());
    } else if (auto t = std::dynamic_pointer_cast<Type_str>(type)) {
        // string type -> treat it as a file include
        std::string file_name = t->cpp_value(value.get_value());
        scope = std::static_pointer_cast<Scope>(parse_file(file_name));
    } else {
        log_error("Mismatched type in using statement: expected either scope or string type, but found type "+type->toS(), us->context);
        us->status = Parsing_status::TYPE_ERROR;
        return us->status;
    }

    ASSERT(scope != nullptr);

    if (is_error(scope->status)) {
        us->status = scope->status;
    } else {
        ASSERT(scope->status == Parsing_status::PARTIALLY_PARSED || scope->status == Parsing_status::FULLY_RESOLVED);
        parent_scope->pulled_in_scopes.push_back(scope);
        us->status = Parsing_status::FULLY_RESOLVED;
    }

    return us->status;
}




void resolve_imports(std::shared_ptr<Scope> scope)
{
    scope->using_statements.size();
    std::vector<std::shared_ptr<Using_statement>> remaining;

    while(scope->using_statements.size() > 0) {
        remaining.clear();

        for (auto& us : scope->using_statements) {
            Parsing_status status = fully_resolve_using(us);
            if (status == Parsing_status::DEPENDENCIES_NEEDED) {
                remaining.push_back(us);
            }
            // if fully resolved, great
            // if error, ignore the scope
            // we could assert that the status is not NOT_PARSED or PARTIALLY_PARSED, but it doesn't seem necessary
        }

        if (scope->using_statements.size() == remaining.size()) {
            for (auto& us : remaining) {
                log_error("Unable to resolve using statement", us->context);
            }
            break;
        }

        scope->using_statements = std::move(remaining);
    }
    scope->using_statements.clear();
}
