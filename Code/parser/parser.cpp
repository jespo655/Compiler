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
    if (global_scopes[name] != nullptr) return global_scopes[name];
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

    auto global_scope = std::shared_ptr<Global_scope>(new Global_scope(tokens));
    Token_iterator it = Token_iterator(tokens);

    auto parsed_scope = std::static_pointer_cast<Parsed_scope>(global_scope);
    partially_parse(it, parsed_scope, false); // static scope -> dynamic = false
    ASSERT(global_scope->status != Parsing_status::NOT_PARSED);

    global_scopes[name] = global_scope;

    return global_scope;
}





std::shared_ptr<Global_scope> get_global_scope(std::shared_ptr<Scope> scope)
{
    if (scope == nullptr) return nullptr;
    if (auto gs = dynamic_pointer_cast<Global_scope>(scope)) return gs;
    auto parent = scope->parent_scope;
    return get_global_scope(parent);
}

template<typename T>
Token_iterator get_iterator(std::shared_ptr<T> tp, int index)
{
    auto gs = dynamic_pointer_cast<Global_scope>(tp);
    if (gs == nullptr) {
        auto abstx = dynamic_pointer_cast<Abstx_node>(tp));
        ASSERT(abstx != nullptr);
        gs = get_global_scope(abstx->parent_scope());
    }
    ASSERT(gs != nullptr);
    return gs->iterator(index);
}





































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








// FIXME: REMOVE BELOW


// --------------------------------------------------------------------------------------------------------------------
//
//          Statement examination
//          Used to determine which type of statement to read
//          Also used to verify that the general code structure is valid, e.g. that parens
//            match and that each statement is ended with a semicolon (if applicable)
//
// --------------------------------------------------------------------------------------------------------------------



// if go_to_next_statement is false, the iterator will be reset to the beginning of the statement
// returns nullptr if no more statements could be read.
std::shared_ptr<Statement> examine_statement(Token_iterator& it, std::shared_ptr<Parsed_scope> parent_scope, bool allow_dynamic, bool go_to_next_statement)
{
    int start_index = it.current_index;
    Token_context context = it->context;
    std::shared_ptr<Statement> statement{nullptr};

    const Token& start_token = it.current_token();

    if (start_token.is_eof()) {
        // no error, just return nullptr as expected
        return nullptr;

    } else if (start_token.type == Token_type::SYMBOL && start_token.token == ";") {
        // empty statement, give warning and ignore
        log_warning("Additional ';' found", start_token.context);
        return examine_statement(it, parent_scope, allow_dynamic, go_to_next_statement);

    } else if (start_token.type != Token_type::STRING && start_token.token == "using") {
        auto us = std::shared_ptr<Using_statement>{new Using_statement()};
        parent_scope->using_statements.push_back(us);
        statement = std::static_pointer_cast<Statement>(us);
        it.current_index = it.find_matching_semicolon(it.current_index) + 1;

    } else if (start_token.type == Token_type::SYMBOL && start_token.token == "{") {
        auto scope = std::shared_ptr<Anonymous_scope>{new Anonymous_scope()};
        scope->start_token_index = it.current_index;
        parent_scope->anonymous_scopes.push_back(scope);
        it.current_index = it.find_matching_brace();
        statement = std::static_pointer_cast<Statement>(scope);

    }

    else if (start_token.type != Token_type::STRING && start_token.token == "if") {
        if (!allow_dynamic) log_error("if statement not allowed in static scope", start_token.context); // FIXME: proper error message
        auto if_s = std::shared_ptr<If_statement>{new If_statement()};
        partially_parse(it, if_s);
        statement = std::static_pointer_cast<Statement>(if_s);
    }

    else if (start_token.type != Token_type::STRING && start_token.token == "for") {
        if (!allow_dynamic) log_error("for statement not allowed in static scope", start_token.context); // FIXME: proper error message
        auto for_s = std::shared_ptr<For_statement>{new For_statement()};
        partially_parse(it, for_s);
        statement = std::static_pointer_cast<Statement>(for_s);
    }

    else if (start_token.type != Token_type::STRING && start_token.token == "while") {
        if (!allow_dynamic) log_error("while statement not allowed in static scope", start_token.context); // FIXME: proper error message
        auto while_s = std::shared_ptr<While_statement>{new While_statement()};
        partially_parse(it, while_s);
        statement = std::static_pointer_cast<Statement>(while_s);
    }

    else if (start_token.type != Token_type::STRING && start_token.token == "return") {
        if (!allow_dynamic) log_error("Return statement not allowed in static scope", start_token.context); // FIXME: proper error message
        statement = std::shared_ptr<Statement>{new Return_statement()};
        it.current_index = it.find_matching_semicolon(it.current_index) + 1;
        if (it.error) statement->status = Parsing_status::FATAL_ERROR;
        else statement->status = Parsing_status::PARTIALLY_PARSED;
    }

    else while (!it.error && !it->is_eof()) {

        // look for ':' (declaration), '=' (assignment) and ';' (unknown statement)
        // the first such matching symbol found determines the type of statement.
        // If an unknown statement ends with ");", then it's a function call

        const Token& t = it.eat_token();

        if (t.type == Token_type::SYMBOL) {

            if (t.token == ":") {
                statement = std::shared_ptr<Statement>(new Declaration_statement());
                auto decl_s = std::shared_ptr<Declaration_statement>{new Declaration_statement()};
                int decl_index = it.current_index-1; // the ':' token
                it.current_index = start_index;
                partially_parse(it, decl_s, decl_index, parent_scope);
                statement = std::static_pointer_cast<Statement>(decl_s);
                break;
            }

            if (t.token == "=") {
                statement = std::shared_ptr<Statement>(new Assignment_statement());
                if (go_to_next_statement) it.current_index = it.find_matching_semicolon(it.current_index) + 1;
                if (it.error) statement->status = Parsing_status::FATAL_ERROR;
                else statement->status = Parsing_status::PARTIALLY_PARSED;
                break;
            }

            if (t.token == ";") {
                const Token& lb2 = it.look_back(2);
                if (lb2.type == Token_type::SYMBOL && lb2.token == ")") {
                    auto fc = std::shared_ptr<Function_call>(new Function_call());
                    auto fc_statement = std::shared_ptr<Function_call_statement>{new Function_call_statement()};
                    fc_statement->function_call = fc;
                    fc->owner = fc_statement;
                    statement = std::static_pointer_cast<Statement>(fc_statement);

                    if (start_token.type == Token_type::COMPILER_COMMAND && start_token.token == "#run") {
                        // FIXME: ensure that #run has the correct case (#RUN / #run)
                        fc->compile_time = true;
                        fc->start_token_index = start_index+1;
                        fc->context = it[start_index+1].context;

                        auto scope = parent_scope->global_scope();
                        if (scope == nullptr) scope = std::static_pointer_cast<Scope>(parent_scope);
                        auto global_scope = std::dynamic_pointer_cast<Global_scope>(scope);
                        ASSERT(global_scope != nullptr);

                        global_scope->run_statements.push_back(fc_statement);
                    } else {
                        fc->compile_time = false;
                        fc->start_token_index = start_index;
                        fc->context = context;
                    }

                } else {
                    statement = std::shared_ptr<Statement>(new Unknown_statement());
                }
                break;
            }

            if (t.token == "(") it.current_index = it.find_matching_paren(it.current_index-1) + 1;  // go back to the previous "(" and search from there
            else if (t.token == "[") it.current_index = it.find_matching_bracket(it.current_index-1) + 1;
            else if (t.token == "{") it.current_index = it.find_matching_brace(it.current_index-1) + 1;

            else if (t.token == ")" || t.token == "]" || t.token == "}") {
                log_error("Unexpected token "+t.token+" in unresolved statement.", t.context);
                statement = std::shared_ptr<Statement>(new Unknown_statement());
                statement->status = Parsing_status::FATAL_ERROR;
                it.error = true;
                break;
            }
        }
    }

    ASSERT(statement != nullptr)

    statement->start_token_index = start_index;
    statement->owner = parent_scope;
    statement->context = context;

    ASSERT(!it.error || statement->status == Parsing_status::FATAL_ERROR);

    if (!go_to_next_statement) {
        it.current_index = start_index;
    }

    return statement;
}





