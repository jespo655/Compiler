#include "parser.h"
#include "error_handler.h"
#include "lexer.h"

using namespace std;

/*

Return bool: true if errors, false if ok
Take Token const* it as first argument
Always check token types for EACH token
    If type==EOF -> eof
    If type==UNKNOWN -> probably error


*/

/*
    TODO:
    read_static_scope has lots of code that could be reused for read_dynamic_scope.
        refactor into read_statement(Token const it*, unique_ptr<Dynamic_statement>& statement, bool force_static)
        ASSERT(Static_statement* ss = dynamic_cast<Static_statement*>(statement))



*/



// lookup functions
// TODO: make these functions a search in a sorted vector of tokens @optimization

bool is_assignment_operator(const Token& t)
{
    if (t.type != Token_type::SYMBOL) return false;

    if (t.token == ":") return true;
    if (t.token == "=") return true;
    if (t.token == "+=") return true;
    if (t.token == "-=") return true;
    if (t.token == "*=") return true;
    if (t.token == "/=") return true;
    if (t.token == "%=") return true;

    return false;
}

bool is_closing_symbol(const Token& t)
{
    if (t.type != Token_type::SYMBOL) return false;

    if (t.token == ")") return true;
    if (t.token == "]") return true;
    if (t.token == "}") return true;

    return false;
}

bool is_primitive_type(const Token& t)
{
    if (t.type != Token_type::IDENTIFIER) return false;

    if (t.token == "u8") return true;
    if (t.token == "u16") return true;
    if (t.token == "u32") return true;
    if (t.token == "u64") return true;
    if (t.token == "i8") return true;
    if (t.token == "i16") return true;
    if (t.token == "i32") return true;
    if (t.token == "i64") return true;
    if (t.token == "int") return true;
    if (t.token == "float") return true;
    if (t.token == "string") return true;

    return false;
}






// start must point at the opening thing
// returns pointers to the closing thing

Token const * read_paren(Token const * it);
Token const * read_bracket(Token const * it);
Token const * read_brace(Token const * it);

Token const * read_token_range_recursive(Token const * it, const string& expected_closing_token, const string& range_name)
{
    while(true) {
        if ((++it)->type == Token_type::EOF) {
            log_error("Missing \""+expected_closing_token+"\" at end of file",it->context);
            return nullptr;
        }
        if (it->type == Token_type::SYMBOL) {
            if (is_closing_symbol(*it)) {
                if (it->token != expected_closing_token) {
                    log_error("Mismatched "+range_name+": expected \""+expected_closing_token+"\" before \""+it->token+"\"",it->context);
                    add_note("In "+range_name+" starting at: ",it->context);
                    return nullptr;
                }
                return it; // ok!
            }
            else if (it->token == "(") it = read_paren(it);
            else if (it->token == "[") it = read_bracket(it);
            else if (it->token == "{") it = read_brace(it);

            if (it == nullptr) return nullptr;
        }
    }
}

Token const * read_paren(Token const * it)
{
    ASSERT(it != nullptr && it->type == Token_type::SYMBOL && it->token == "(");
    return read_token_range_recursive(it,")","paren");
}

Token const * read_bracket(Token const * it)
{
    ASSERT(it != nullptr && it->type == Token_type::SYMBOL && it->token == "[");
    return read_token_range_recursive(it,"]","bracket");
}

Token const * read_brace(Token const * it)
{
    ASSERT(it != nullptr && it->type == Token_type::SYMBOL && it->token == "{");
    return read_token_range_recursive(it,"}","brace");
}























/*
void add_dependency(Dynamic_statement* statement, Identifier* identifier, Scope* scope)
{
    if (Static_scope* ss = dynamic_cast<Static_scope*>(scope.get())) {
        ss->dependencies.push_back(pair(statement,identifier));
    } else {
        for (Scope* s : scope.imported_scopes) {
            add_dependency(statement,identifier,s);
        }
    }
}

void remove_dependancies(Dynamic_statement* statement, Scope* scope)
{
    if (Static_scope* ss = dynamic_cast<Static_scope*>(scope.get())) {
        for (int i = ss->dependencies.size()-1; i >= 0; --i) {
            if (ss->dependencies[i].first == statement) {
                ss->dependencies.erase(i);
            }
        }
    } else {
        for (Scope* s : scope.imported_scopes) {
            remove_dependancies(statement,s);
        }
    }
}

void resolve_identifier(Typed_identifier* identifier, Scope* scope)
{
    for (Typed_identifier& ids : scope.identifiers) {
        // check if it's alread in the scope. If not -> add it.
        // If there - assert that it not already has a type - This function should only be called once per identifier.
    }
    vector<Dynamic_statement*> statements_to_be_resolved{};
    if (Static_scope* ss = dynamic_cast<Static_scope*>(scope.get())) {
        for (auto p : ss->dependencies) {
            // if the identifier match -> reduce the statement's dependency count by 1
            // if dep.count reaches 0 -> add to statements_to_be_resolved
        }
    }
    for (Dynamic_statement* ds : statements_to_be_resolved) {
        ASSERT(ds->dependencies == 0);
        // Try to resolve. If it's not possible -> some error in the compiler
    }
}
*/


// TODO: check this case:
/*
s1:={
    a : int;
    s2:={
        b := a;     // when we reach this point, a will be defined in s1 but not s2.
        a : float;  // this is closer to b than the previous definition -> b should be a float!
    };
};
*/

// Try to resolve the local scope first. If fully resolved and identifier not found -> add dependencies on other scopes
//      Check level for level
//      Add identifiers to the scope even if they can't be resolved, so we know they exist
// Go through all scopes and set up dependencies and add all identifiers BEFORE resolving depending statements
// -> When setting up dependencies, we can KNOW which scope we want to get the identifier from
// Can also give error if we find multiple declarations of the same identifier on the same level







/*
1) Go through all static scopes in the workspace
    add identifiers to the scopes
    set up dependencies only in the local scope
    store a list of dynamic scopes to compile (the bodies of functions)

2) Import scopes with "using" (has to be done before step 3)

3) (loop) Resolve unresolved identifiers.

*/



























// type_token: ":"-token
// assignment_token: "="-token, or other assignment operators if
// end_token: ";"-token
// returns true if error
bool examine_statement(Token const* it, Token const*& type_token, Token const*& assignment_token, Token const*& end_token)
{
    ASSERT(it != nullptr);

    bool found_assignment = false;
    bool found_type = false;
    bool errors = false;

    while(true) {
        if (it->type == Token_type::EOF || is_closing_symbol(*it)) {
            log_error("Missing \";\" after statement",it->context);
            add_note("Expected \";\" before \""+it->token+"\"");
            return errors;
        }
        if (it->type == Token_type::SYMBOL) {
            if (it->token == ";") {
                end_token = it;
                return errors;
            }
            else if (it->token == "(") it = read_paren(it);
            else if (it->token == "[") it = read_bracket(it);
            else if (it->token == "{") it = read_brace(it);
            else if (it->token == ":") {
                if (found_type) {
                    log_error("Multiple \":\" operators in statement",it->context);
                    add_note("First found here: ",assignment_token->context);
                    add_note("Expected \";\" after each assignment.");
                    errors = true;
                }
                if (found_assignment) {
                    log_error("Unexpected \":\" operator found after assignment.",it->context);
                    add_note("Assignment operator found here: ",assignment_token->context);
                    add_note("\":\" must always come before \"=\".");
                    errors = true;
                }
                found_type = true;
                type_token = it;
            }
            else if (is_assignment_operator(*it)) {
                if (found_assignment) {
                    log_error("Multiple assignment operators in statement",it->context);
                    add_note("First found here: ",assignment_token->context);
                    add_note("Expected \";\" after each assignment.");
                    errors = true;
                }
                if (found_type && it->token != "=") {
                    log_error("Unexpected assignment token: \""+it->token+"\"",it->context);
                    add_note("Only \"=\" are allowed after \":\".");
                    add_note("\":\" token found here: ",type_token->context);
                    errors = true;
                }
                found_assignment = true;
                assignment_token = it;
            }
            if (it == nullptr) return true; // can't continue after error from read_paren and Co.
        }
        it++;
    }
}


bool read_evaluated_variable(Token const*& it, unique_ptr<Evaluated_variable>& variable, Scope* scope, vector<Dependency>& dependencies);
bool read_evaluated_value(Token const*& it, unique_ptr<Evaluated_value>& value, Scope* scope, vector<Dependency>& dependencies);


// variable:
//      in: the function identifier
//      out: the function itself
bool read_function_call(Token const*& it, unique_ptr<Evaluated_variable>& variable, Scope* scope, vector<Dependency> dependencies)
{
    ASSERT(it != nullptr && it->type == Token_type::SYMBOL && it->token == "(");
    unique_ptr<Function_call> fc{new Function_call()};
    fc->function_identifier = move(variable);

    if (!((++it)->type == Token_type::SYMBOL && it->token == ")")) { // if it's not an empty variable list
        while (true) {
            if (it->type == Token_type::EOF) {
                log_error("Unexpected end of file in function call: expected argument",it->context);
                return true;
            }
            unique_ptr<Evaluated_value> argument;
            if (read_evaluated_value(it,argument,scope,dependencies)) return true; // error

            fc->arguments.push_back(move(argument));
            if ((++it)->type != Token_type::SYMBOL) {
                log_error("Missing \")\" at the end of function call argument list",it->context);
                return true;
            }
            if (it->token == ")") break; // ok

            if (it->token != ",") {
                log_error("Unexpected token in function call argument list. Expected \",\" between arguments",it->context);
                return true;
            }
            ++it; // go past the "," token
        }
    }

    ASSERT(it->type == Token_type::SYMBOL && it->token == ")");

    ++it; // go past the ")" token
    variable = move(fc);
    return false; // ok!
}

// variable:
//      in: the struct identifier
//      out: the getter itself
bool read_getter(Token const*& it, unique_ptr<Evaluated_variable>& variable, Scope* scope, vector<Dependency> dependencies)
{
    ASSERT(it != nullptr && it->type == Token_type::SYMBOL && it->token == ".");
    unique_ptr<Getter> g{new Getter()};
    g->struct_identifier = move(variable);

    if ((++it)->type == Token_type::IDENTIFIER) {
        g->data_identifier_token = it;
        variable = move(g);
        it++; // go past the identifier token
        return false; // ok!
    } else {
        log_error("Expected data identifier after getter token \".\"",it->context);
        return true;
    }
}

// variable:
//      in: the casted value
//      out: the cast itself
bool read_cast(Token const*& it, unique_ptr<Evaluated_value>& variable, Scope* scope, vector<Dependency> dependencies)
{
    ASSERT(it != nullptr && it->type == Token_type::SYMBOL && it->token == "_");
    unique_ptr<Cast> cast{new Cast()};
    cast->casted_value = move(variable);

    if ((++it)->type == Token_type::IDENTIFIER) {
        if (read_evaluated_variable(it,cast->casted_type,scope,dependencies)) return true; // error
        variable = move(cast);
        return false; // ok!
    } else {
        log_error("Expected type identifier after cast token \"_\"",it->context);
        return true;
    }
}
bool read_cast(Token const*& it, unique_ptr<Evaluated_variable>& variable, Scope* scope, vector<Dependency> dependencies)
{
    unique_ptr<Evaluated_value> value = move(variable);
    if (read_cast(it,value,scope,dependencies)) return true; // error
    ASSERT(dynamic_cast<Evaluated_variable*>(value.get()) != nullptr);
    variable.reset(static_cast<Evaluated_variable*>(value.release()));
}


// variable:
//      in: the array identifier
//      out: the lookup itself
bool read_array_lookup(Token const*& it, unique_ptr<Evaluated_variable>& variable, Scope* scope, vector<Dependency> dependencies)
{
    ASSERT(it != nullptr && it->type == Token_type::SYMBOL && it->token == "[");
    unique_ptr<Array_lookup> al{new Array_lookup()};
    al->array_identifier = move(variable);

    if (read_evaluated_value((++it),al->position,scope,dependencies)) return true; // error
    if (it->type != Token_type::SYMBOL || it->token != "]") {
        log_error("Missing \"]\" at the end of array lookup",it->context);
        return true;
    }
    ++it; // go past the "]" token
    variable = move(al);
    return false;
}



bool read_call_chain(Token const*& it, unique_ptr<Evaluated_variable>& variable, Scope* scope, vector<Dependency>& dependencies)
{
    while(++it) {
        if (it->type != Token_type::SYMBOL) return false; // end of evaluated_variable
        if (it->token == "(") {
            // read function call
            if (read_function_call(it,variable,scope,dependencies)) return true; // error
            ASSERT((it-1)->type == Token_type::SYMBOL && (it-1)->token == ")"); // should be the end of the function call
        } else if (it->token == ".") {
            // read getter
            if (read_getter(it,variable,scope,dependencies)) return true;
        } else if (it->token == "_") {
            // read cast
            if (read_cast(it,variable,scope,dependencies)) return true;
        } else if (it->token == "[") {
            // read array lookup
            if (read_array_lookup(it,variable,scope,dependencies)) return true;
            ASSERT((it-1)->type == Token_type::SYMBOL && (it-1)->token == "]"); // should be the end of the array lookup
        }
    }
}



// TODO: add dependencies
bool read_evaluated_variable(Token const*& it, unique_ptr<Evaluated_variable>& variable, Scope* scope, vector<Dependency>& dependencies)
{
    // read variable. For anything other than regular identifiers, add dependencies to the list (not to the scope)
    // later: in declaration context: ensure that this is a variable and it is not already in the scope

    // has to start with an IDENTIFIER token. Everything else is pure-rhs.
    // TODO: allow dump variable @dump
    ASSERT(it != nullptr);

    if (it->type != Token_type::IDENTIFIER) {
        log_error("Unexpected token: expected identifier",it->context);
        return true; // error
    }

    unique_ptr<Identifier> id{new Identifier()};
    id->identifier_token = it;
    variable = move(id);
    read_call_chain(it,variable,scope,dependencies);
    return false;
}




bool read_value_list(Token const*& it, unique_ptr<Evaluated_value>& value, Scope* scope, vector<Dependency>& dependencies)
{
    cerr << "read_value_list not yet implemented" << endl;
    return true;
}



// TODO: add dependencies
// TODO: read infix op
bool read_evaluated_value(Token const*& it, unique_ptr<Evaluated_value>& value, Scope* scope, vector<Dependency>& dependencies)
{
    // read value. Except for Evaluated_variables, this can also be literals: string, integer, float, bool, array

    ASSERT(it != nullptr);
    value = nullptr;

    if (it->type == Token_type::IDENTIFIER) {
        unique_ptr<Evaluated_variable> variable;
        if (read_evaluated_variable(it,variable,scope,dependencies)) return true; // error
        value = move(variable);
        return false;
    }

    // Except for an Evaluated_variable, a value can start with:
    //      1) any literal
    //      2) a parenthesis of comma separated Evaluated_values

    // Continuations must start with a cast "_", but can after that be used as a variable

    if (it->type == Token_type::STRING ||
        it->type == Token_type::INTEGER ||
        it->type == Token_type::FLOAT ||
        it->type == Token_type::BOOL)
    {
        unique_ptr<Literal> literal{new Literal()};
        literal->literal_token = it;
        value = move(literal);
        ++it;
    } else if (it->type == Token_type::SYMBOL && it->token == "(") {
        // read Value_list
        if (read_value_list(it,value,scope,dependencies)) return true;
        ASSERT(value != nullptr);
        ASSERT(it->type == Token_type::SYMBOL && it->token == ")");
        it++; // go past the ")" token
    }

    // if something else than "_" -> return
    // else, read the cast. That transforms the value into a variable -> can go into the next loop:
    if (it->type == Token_type::SYMBOL && it->token == "_") {
        if (read_cast(it,value,scope,dependencies)) return true;
        ASSERT(dynamic_cast<Evaluated_variable*>(value.get()) != nullptr);
        unique_ptr<Evaluated_variable> variable{static_cast<Evaluated_variable*>(value.release())};
        read_call_chain(it,variable,scope,dependencies);
        value = move(variable);
    }

    return false;
}












// bool resolve_evaluated_variable()


// returns true if errors
// returns false if syntax seems good, even if not able to deduce types yet. In that case, dependencies are added.
bool read_declaration(Token const* it, Token const * type_token, Token const * assignment_token, Token const * end_token, Scope* scope, bool force_static)
{
    ASSERT(type_token->type == Token_type::SYMBOL);
    ASSERT(type_token->token == ":");
    // Read declaration

    unique_ptr<Declaration> declaration{new Declaration()};
    bool errors = false;

    // TODO
    // read lhs
    cerr << "should read lhs of declaration, but not yet implemented" << endl;

    Token const* type_end = end_token;
    if (assignment_token != nullptr) type_end = assignment_token;

    if (type_end > type_token+1) {

        // TODO
        // Read the type identifiers. Add them to the scope if not already added.
        // Update the corresponding identifiers with type info.
        cerr << "should read type identifier list, but not yet implemented" << endl;

    } else if (assignment_token == nullptr) {
        log_error("Unable to infer the types of identifiers; missing type identifiers after \":\"",end_token->context);
        errors = true;
    }

    if (assignment_token != nullptr) {
        ASSERT(assignment_token->type == Token_type::SYMBOL);
        ASSERT(assignment_token->token == "=");

        // TODO
        // Also read rhs. Enforce static_scope.
        cerr << "should read rhs of declaration, but not yet implemented" << endl;
    }

    // TODO: add all defined identifiers to the scope
    // if they are already defined -> error
    // that should be done even if we are unable to infer the type of the variable
    // scope->statements.push_back(declaration); // TODO
    return errors;
}



// dynamic scopes will be processed AFTER all static scopes
// -> we know that all identifiers are declared
bool read_assignment(Token const * it, Token const * assignment_token, Token const * end_token, Dynamic_scope* scope)
{
    ASSERT(assignment_token != nullptr);
    ASSERT(is_assignment_operator(*assignment_token));

    cerr << "read_assignment not yet implemented" << endl;
    return false;

    // read lhs
    // ensure that we can find all identifiers. If not -> error!

    // read rhs
    // ensure that we can find all identifiers. If not -> error!

    // set up dependencies for each unresolved identifier in lhs or rhs.
    // if no dependencies -> resolve_assignment() immediately!
}

bool resolve_assignment()
{
    // Check that the types between lhs and rhs match. This might not be resolved.
    // If all types are resolved -> give an error for each type mismatch
}



bool handle_imports(Token const *& it, Scope* scope, Scope* parent_scope)
{
    ASSERT(it != nullptr);
    ASSERT(scope != nullptr);
    ASSERT(parent_scope != nullptr);

    if (it->type == Token_type::SYMBOL && it->token == "[") {
        // read capture group
        it = read_bracket(it);
        if (it == nullptr) return true; // error and can't continue
        it++; // go to the "{" token
        cerr << "reading capture group not yet implemented" << endl;
    } else {
        // import parent scope
        scope->imported_scopes.push_back(parent_scope);
    }
    return false; // no error
}


bool read_static_scope(Token const * it, unique_ptr<Static_scope>& scope, Static_scope* parent_scope)
{
    ASSERT(it != nullptr);
    ASSERT(parent_scope != nullptr);

    scope.reset(new Static_scope());

    if (handle_imports(it, scope.get(), parent_scope)) {
        return true; // error
    }

    ASSERT(it->type == Token_type::SYMBOL);
    ASSERT(it->token == "{");

    // read static statements to scope
    cerr << "reading static scope not yet implemented" << endl;
}

// can be a function body, if statement, etc.
bool read_dynamic_scope(Token const * it, unique_ptr<Dynamic_scope>& scope, Scope* parent_scope)
{
    ASSERT(it != nullptr);
    ASSERT(parent_scope != nullptr);

    scope.reset(new Dynamic_scope());

    if (handle_imports(it, scope.get(), parent_scope)) {
        return true; // error
    }

    ASSERT(it->type == Token_type::SYMBOL);
    ASSERT(it->token == "{");

    // read dynamic statements to scope
    cerr << "reading dynamic scope not yet implemented" << endl;
}












// stops when there are no more ";"-tokens in the current scope
// returns true if error
bool read_static_scope_statements(Token const* it, Static_scope* scope)
{
    ASSERT(it != nullptr);

    bool errors = false;

    while(it->type != Token_type::EOF && !is_closing_symbol(*it)) {

        Token const* type_token = nullptr;
        Token const* assignment_token = nullptr;
        Token const* end_token = nullptr;
        if (examine_statement(it, type_token, assignment_token, end_token)) {
            // We encountered an error, but if possible, continue going for the rest of the statements.
            // Getting several errors at the same time makes debugging easier.
            if (end_token == nullptr) return true; // not possible to go further
            it = end_token+1;
            errors = true;
            continue;
        }

        /**
            Possibilities:

            1) only ":" token, no assignment
            2) both ":" and "=" tokens
            3) only assignment token
            4) no ":" nor assignment tokens
        */
        if (type_token != nullptr) {
            ASSERT(type_token->type == Token_type::SYMBOL);
            ASSERT(type_token->token == ":");
            read_declaration(it,type_token,assignment_token,end_token, scope, true);

        } else if (assignment_token != nullptr) {
            ASSERT(assignment_token->type == Token_type::SYMBOL);
            ASSERT(is_assignment_operator(*assignment_token));

            // Assignment

            log_error("Variable assignment not allowed in a static scope!",assignment_token->context);
            errors = true;
        } else {

            // Can be a function call or an anonymous scope
            unique_ptr<Evaluated_value> value{nullptr};
            vector<Dependency> dependencies{}; // TODO: add these to the scope
            if (read_evaluated_value(it,value,scope,dependencies)) {
                errors = true;
            } else {
                ASSERT(value != nullptr);
                if (Scope* s = dynamic_cast<Scope*>(value.get())) {
                    cerr << "read anonymous scope, but don't know how to handle it yet" << endl;
                } else if (Function_call* fc = dynamic_cast<Function_call*>(value.get())) {
                    log_error("Function call not allowed in static scope!",it->context);
                    add_note("Use \"#run\" to create a dynamic scope (NYI)");
                    errors = true;
                }
            }
        }
        it = end_token+1;
    }
    return errors;
}












std::unique_ptr<Scope> parse_tokens(const std::vector<Token>& tokens)
{
    cerr << "parse_tokens not yet implemented" << endl;
    return nullptr;
    /*
    if (tokens.empty()) return nullptr;
    unique_ptr<Scope> global_scope{new Scope()};
    global_scope->statements = read_statement_list(tokens,&tokens[0],global_scope.get(),false);
    if (global_scope->statements.empty()) {
        // since t is not empty we expect at least one statement. -> errors -> return nulllptr.
        return nullptr;
    }
    Token const * t = global_scope->statements.back()->end_token+2; // +1 is ";" token. +2 should be eof
    if (t < &tokens.back()) {
        log_error("Extra token after statements in global scope: Found extra token \""+t->token+"\"",t->context);
        return nullptr;
    }
    global_scope->start_token = &tokens[0];
    global_scope->end_token = &tokens.back();
    return global_scope;
    */
}

std::unique_ptr<Scope> parse_file(const std::string& file)
{
    return parse_tokens(get_tokens_from_file(file));
}

std::unique_ptr<Scope> parse_string(const std::string& string)
{
    return parse_tokens(get_tokens_from_string(string));
}
