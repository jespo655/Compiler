#include "parser.h"
#include "../abstx/declaration.h"


// syntax:
// a, b, c := expr;
// a, b, c : expr = expr;
std::shared_ptr<Declaration_statement> read_declaration_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(parent_scope != nullptr && !parent_scope->dynamic); // if dynamic, should use compile_declaration_statement instead

    // expect identifier token
    // optional, read ',' then expect another identifier token
    // optional, read exactly one identifier or symbol token (should be infix operator, but we can't verify that yet)
    // expect ':' token
    // find end of statement (';')

    auto decl = std::shared_ptr<Declaration_statement>(new Declaration_statement());
    decl->start_token_index = it.current_index;
    decl->context = it->context;
    decl->owner = parent_scope;

    while(true) {

        it.expect_current(Token_type::IDENTIFIER);
        if (it.error) {
            decl->status = Parsing_status::SYNTAX_ERROR;
            break;
        } else {
            // check if the identifier already exists
            auto old_id = parent_scope->get_identifier(it->token, false);
            if (old_id != nullptr) {
                log_error("Redeclaration of identifier "+it->token, it->context);
                add_note("Previously declared here: ", old_id->context);
                // no specific error in the declaration, just ignore the identifier
                it.eat_token(); // eat the identifier token
            } else {
                auto id = read_identifier(it, parent_scope);
                id->owner = decl;
                decl->identifiers.push_back(id);
                parent_scope->identifiers[id->name] = id;
            }
        }

        if (it->type == Token_type::SYMBOL && it->token == ":")
            break;

        it.expect(Token_type::SYMBOL, ",");
        if (it.error) {
            decl->status == Parsing_status::SYNTAX_ERROR;
            break;
        }
    }

    it.current_index = it.find_matching_semicolon() + 1;
    if (it.error) decl->status == Parsing_status::FATAL_ERROR;
    else if (!is_error(decl->status)) decl->status = Parsing_status::PARTIALLY_PARSED;

    return decl;

}





Parsing_status fully_resolve_declaration(std::shared_ptr<Declaration_statement>& declaration)
{
    ASSERT(false, "NYI");
}

std::shared_ptr<Declaration_statement> compile_declaration_statement(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(false, "NYI");
}













/*
    // For allowing infix operators in assignment:
        if (it->type == Token_type::IDENTIFIER || it->type == Token_type::SYMBOL) {
            Token& t = it.look_ahead(1));
            if (t.type == Token_type::SYMBOL && t.token == ":")
                break; // treat the identifier or symbol token as an infix operator
        }
*/