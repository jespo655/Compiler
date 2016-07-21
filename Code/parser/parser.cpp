#include "parser.h"
#include "lexer.h"
#include "parsing_status.h"
#include "token_iterator.h"

#include "../abstx/scope.h"
#include "../abstx/using.h"
#include "../abstx/statement.h"
#include "../abstx/if.h"
#include "../abstx/for.h"
#include "../abstx/while.h"
#include "../abstx/function.h"
#include "../abstx/declaration.h"
#include "../abstx/assignment.h"
#include "../abstx/return.h"

#include <map>




std::map<std::string, std::shared_ptr<Global_scope>> global_scopes;



// FIXME: store a map name -> global scope with parsed scopes
// If the name already has a global scope, return that instead.
std::shared_ptr<Global_scope> parse_file(const std::string& file)
{
    if (global_scopes[file] != nullptr) return global_scopes[file];
    return parse_tokens(get_tokens_from_file(file), file);
}

std::shared_ptr<Global_scope> parse_string(const std::string& string, const std::string& name) // FIXME: add string context
{
    ASSERT(string != "");
    ASSERT(name != "");
    if (global_scopes[name] != nullptr) return global_scopes[name];
    return parse_tokens(get_tokens_from_string(string), name);
}

std::shared_ptr<Global_scope> parse_tokens(const std::vector<Token>& tokens, const std::string& name)
{
    ASSERT(name != "");
    if (global_scopes[name] != nullptr) return global_scopes[name];
    if (tokens.size() <= 1) return nullptr;

    auto global_scope = read_global_scope(tokens, name);

    ASSERT(global_scope->status != Parsing_status::NOT_PARSED);
    if (!is_error(global_scope->status)) {
        global_scopes[name] = global_scope;
    }

    return global_scope;
}





std::shared_ptr<Global_scope> get_global_scope(std::shared_ptr<Scope> scope)
{
    if (scope == nullptr) return nullptr;
    if (auto gs = std::dynamic_pointer_cast<Global_scope>(scope)) return gs;
    auto parent = scope->parent_scope();
    return get_global_scope(parent);
}

template<typename T>
Token_iterator get_iterator(std::shared_ptr<T> tp, int index)
{
    auto gs = std::dynamic_pointer_cast<Global_scope>(tp);
    if (gs == nullptr) {
        auto abstx = std::dynamic_pointer_cast<Abstx_node>(tp));
        ASSERT(abstx != nullptr);
        gs = get_global_scope(abstx->parent_scope());
    }
    ASSERT(gs != nullptr);
    return gs->iterator(index);
}

































/*



// --------------------------------------------------------------------------------------------------------------------
//
//          Parsing pass 2: Stepping through #runs, fully resolving everything reached.
//          This parsing is recursive.
//          The only special case is function calls - the function only has to be partially parsed
//            for the caller to be classified as fully parsed.
//
//          FIXME: watch out for cyclic dependencies
//
// --------------------------------------------------------------------------------------------------------------------



// FIXME: move to declaration_parser
Parsing_status fully_resolve(std::shared_ptr<Declaration_statement>& declaration)
{
    // TODO:
    // it's partially parsed up till the ':' token
    // After that: check for an optional list of type identifiers
    // If no type list, expect '=' token
    // If type list, update the types of the identifiers, then check for optional '=' token
    // If '=' token, expect a list of value expressions
    // Fully resolve them and store their return values in a vector
    // If type list, check the vector against the types, ensure they match
    // If no type list, update the types of the identifiers
    // Expect ';'

    return declaration->status;
}


// FIXME: move to assignment_parser
Parsing_status fully_resolve(std::shared_ptr<Assignment_statement>& assignment)
{
    // TODO:
    // Read and fully resolve variable expressions up to the '=' token, store their types in a vector
    // Read and fully resolve value expressions up to the ';' token, store their types in a vector
    // Ensure that the types match
}























// --------------------------------------------------------------------------------------------------------------------
//
//          Parsing pass 1: Partial parsing of scopes
//          Statements are examined and stored in a list, partially parsed
//          3 kinds of statements have special treatment after all statements are added. In order:
//            1) using statements are fully resolved, and the corresponding scope is partially parsed and then imported
//            2) anonymous scopes are partially parsed (repeating these steps)
//            3) #run statements are stored in a list in the global scope in preparation for pass 2.
//
// --------------------------------------------------------------------------------------------------------------------


// FIXME: move to using_parser
Parsing_status fully_resolve(Token_iterator& it, std::shared_ptr<Using_statement>& statement)
{
    if (statement->status == Parsing_status::FULLY_RESOLVED || is_error(statement->status)) {
        return statement->status;
    }

    it.current_index = statement->start_token_index;
    it.error = false;

    ASSERT(statement->status == Parsing_status::PARTIALLY_PARSED);
    ASSERT(it->type == Token_type::KEYWORD && it->token == "using");

    it.eat_token(); // eat the "using" token

    std::shared_ptr<Scope> parent_scope = statement->parent_scope();
    ASSERT(parent_scope != nullptr);

    std::shared_ptr<Scope> scope;
    std::string key = "";

    const Token& id_token = it.eat_token();
    if (id_token.type == Token_type::STRING) {
        it.expect(Token_type::SYMBOL, ";");
        if (it.error) statement->status = Parsing_status::SYNTAX_ERROR;
        else {
            key = "_file_"+id_token.token;
            if(parent_scope->pulled_in_scopes[key] == nullptr)
                scope = parse_file(id_token.token);

            statement->status = Parsing_status::FULLY_RESOLVED;
        }

    } else if (id_token.type == Token_type::IDENTIFIER) {
        it.expect(Token_type::SYMBOL, ";");
        if (it.error) statement->status == Parsing_status::SYNTAX_ERROR;
        else {
            key = "_id_"+id_token.token;

            // FIXME: pull in scope from identifier
            // Fully resolve identifier
            // If type is not Type_scope, then log error
            // Else find its identity
            // If the identifier could not be resolved, return Parsing_status::DEPENDENCIES_NEEDED
            // statement->status = Parsing_status::DEPENDENCIES_NEEDED;
            log_error("Including scopes by name not yet implemented", id_token.context);
            add_note("In the meantime, put the scope in a separate file and include by filename");
            statement->status = Parsing_status::SYNTAX_ERROR;
        }

    } else {
        log_error("Unexpected token in using statement: expected a string literal (filename) or a scope identifier but found \""
            +id_token.token+"\" ("+toS(id_token.type)+")", id_token.context);
        statement->status = Parsing_status::SYNTAX_ERROR;
    }

    if (scope != nullptr) {
        ASSERT(key != "");
        ASSERT(parent_scope->pulled_in_scopes[key] == nullptr)
        parent_scope->pulled_in_scopes[key] = scope;
    }
    return statement->status;
}





















// --------------------------------------------------------------------------------------------------------------------
//
//          Partial parsing of statements
//          Used to extract immediately obvious data
//
// --------------------------------------------------------------------------------------------------------------------



// Partial parse of declaration statement
Parsing_status partially_parse(Token_iterator& it, std::shared_ptr<Declaration_statement>& statement, int declaration_index, std::shared_ptr<Scope> parent_scope)
{
    ASSERT (!it.error);
    ASSERT (it[declaration_index].type == Token_type::SYMBOL && it[declaration_index].token == ":");
    ASSERT (it.current_index < declaration_index);

    statement = std::shared_ptr<Declaration_statement>{new Declaration_statement()};

    bool first = true;

    while(!it.error && it.current_index < declaration_index) {

        if (!first) it.expect(Token_type::SYMBOL, ",");
        if (it.error) break;

        const Token& identifier_token = it.expect(Token_type::IDENTIFIER);

        if (!it.error) {
            auto id = std::shared_ptr<Identifier>(new Identifier());
            id->name = identifier_token.token;
            id->context = identifier_token.context;
            id->owner = statement;
            id->status = Parsing_status::PARTIALLY_PARSED;
            id->start_token_index = it.current_index-1;

            statement->identifiers.push_back(id);
            parent_scope->identifiers[id->name] = id;
        }

        if (it.current_index == declaration_index) break;

        first = false;
    }

    if (it.error) statement->status = Parsing_status::FATAL_ERROR;
    else statement->status = Parsing_status::PARTIALLY_PARSED;

    return statement->status;
}





// Partial parse of for statement
Parsing_status partially_parse(Token_iterator& it, std::shared_ptr<For_statement>& statement)
{
    ASSERT (it->type != Token_type::STRING && it->token == "for");
    ASSERT (!it.error);

    statement = std::shared_ptr<For_statement>{new For_statement()};

    it.eat_token(); // eat the "for" token

    it.expect(Token_type::SYMBOL, "(");
    if (!it.error) it.current_index = it.find_matching_paren(it.current_index-1) + 1; // go back to the previous "(" and search from there

    statement->scope = std::shared_ptr<Scope>{new Scope()};
    statement->scope->owner = statement;
    statement->scope->start_token_index = it.current_index;

    if (!it.error) it.expect(Token_type::SYMBOL, "{");
    if (!it.error) it.current_index = it.find_matching_brace(it.current_index-1) + 1; // same as above

    if (it.error) statement->status = Parsing_status::FATAL_ERROR;
    else statement->status = Parsing_status::PARTIALLY_PARSED;

    return statement->status;
}




// Partial parse of while statement
Parsing_status partially_parse(Token_iterator& it, std::shared_ptr<While_statement>& statement)
{
    ASSERT (it->type != Token_type::STRING && it->token == "while");
    ASSERT (!it.error);

    statement = std::shared_ptr<While_statement>{new While_statement()};

    it.eat_token(); // eat the "while" token

    it.expect(Token_type::SYMBOL, "(");
    if (!it.error) it.current_index = it.find_matching_paren(it.current_index-1) + 1; // go back to the previous "(" and search from there

    statement->scope = std::shared_ptr<Scope>{new Scope()};
    statement->scope->owner = statement;
    statement->scope->start_token_index = it.current_index;

    if (!it.error) it.expect(Token_type::SYMBOL, "{");
    if (!it.error) it.current_index = it.find_matching_brace(it.current_index-1) + 1; // same as above

    if (it.error) statement->status = Parsing_status::FATAL_ERROR;
    else statement->status = Parsing_status::PARTIALLY_PARSED;

    return statement->status;
}




// Partial parse of if statement
Parsing_status partially_parse(Token_iterator& it, std::shared_ptr<If_statement>& statement)
{
    ASSERT (it->type != Token_type::STRING && it->token == "if");
    ASSERT (!it.error);

    statement = std::shared_ptr<If_statement>{new If_statement()};

    // read any number of conditional scopes (if, elsif, elsif, ...)
    do {
        it.eat_token(); // eat the "if"/"elsif" token

        auto cs = std::shared_ptr<Conditional_scope>{new Conditional_scope()};
        cs->owner = statement;
        cs->start_token_index = it.current_index;
        statement->conditional_scopes.push_back(cs);

        it.expect(Token_type::SYMBOL, "(");
        if (!it.error) it.current_index = it.find_matching_paren(it.current_index-1) + 1; // go back to the previous "(" and search from there

        cs->scope = std::shared_ptr<Scope>{new Scope()};
        cs->scope->owner = cs;
        cs->scope->start_token_index = it.current_index;

        if (!it.error) it.expect(Token_type::SYMBOL, "{");
        if (!it.error) it.current_index = it.find_matching_brace(it.current_index-1) + 1; // same as above

    } while (!it.error && it->type != Token_type::STRING && it->token == "elsif");

    if (!it.error && it->type != Token_type::STRING && it->token == "else") {

        it.eat_token(); // eat the "else" token

        statement->else_scope = std::shared_ptr<Scope>{new Scope()};
        statement->else_scope->owner = statement;
        statement->else_scope->start_token_index = it.current_index;

        if (!it.error) it.expect(Token_type::SYMBOL, "{");
        if (!it.error) it.current_index = it.find_matching_brace(it.current_index-1) + 1; // same as above
    }

    if (!it.error && it->type != Token_type::STRING && it->token == "then") {

        it.eat_token(); // eat the "then" token

        statement->then_scope = std::shared_ptr<Scope>{new Scope()};
        statement->then_scope->owner = statement;
        statement->then_scope->start_token_index = it.current_index;

        if (!it.error) it.expect(Token_type::SYMBOL, "{");
        if (!it.error) it.current_index = it.find_matching_brace(it.current_index-1) + 1; // same as above
    }

    if (it.error) statement->status = Parsing_status::FATAL_ERROR;
    else statement->status = Parsing_status::PARTIALLY_PARSED;

    return statement->status;
}






*/



