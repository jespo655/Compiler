#include "parser.h"


// FIXME: read dump variable _

// syntax:
// identifier
// (one token of type Token_type::IDENTIFIER)
std::shared_ptr<Identifier> read_identifier(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(it->type == Token_type::IDENTIFIER);
    auto id = std::shared_ptr<Identifier>(new Identifier());
    id->owner = parent_scope;
    id->context = it->context;
    id->start_token_index = it.current_index;
    id->name = it->token;
    id->status = Parsing_status::PARTIALLY_PARSED;

    it.eat_token(); // eat the identifier token
    return id;
}






Parsing_status fully_resolve_identifier(std::shared_ptr<Identifier>& identifier)
{
    ASSERT(identifier != nullptr);
    if (is_error(identifier.status) || identifier.status == Parsing_status::FULLY_RESOLVED) {
        // no use doing anything
        return identifier.status;
    }

    // find the identity from the parent scope
    // If unable to find it, the identifier is undeclared.
    auto parent_scope = identifier->parent_scope();
    ASSERT(parent_scope != nullptr);

    ASSERT(identifier->name != "");
    auto identity = parent_scope->get_identifier(identifier->name);

    if (identity == nullptr) {
        log_error("Undeclared identifier \""+identifier->name+"\"", identifier->context);
        identifier->status = Parsing_status::UNDECLARED_IDENTIFIER;
        return identifier->status;
    }
    identifier = identity;

    if (identifier->status == Parsing_status::FULLY_RESOLVED || is_error(identifier->status))
        return identifier->status;

    ASSERT(identifier->owner.lock() != nullptr);
    auto declaration = std::dynamic_pointer_cast<Declaration_statement>(identifier->owner.lock());
    ASSERT(declaration != nullptr);

    fully_resolve_declaration(declaration);
    identifier->status = declaration->status;
    ASSERT(identifier->status != Parsing_status::NOT_PARSED);
    ASSERT(identifier->status != Parsing_status::PARTIALLY_PARSED);
    return identifier->status;
}