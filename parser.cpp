#include "parser.h"
#include "error_handler.h"
#include "lexer.h"
#include <sstream>

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




const Token COMMA = Token{Token_type::SYMBOL, ","};
const Token SEMICOLON = Token{Token_type::SYMBOL, ";"};
const Token COLON = Token{Token_type::SYMBOL, ":"};
const Token EOF = Token{Token_type::EOF, "eof"};

const Token FUNCTION_KEYWORD = Token{Token_type::KEYWORD, "fn"};
const Token STRUCT_KEYWORD = Token{Token_type::KEYWORD, "struct"};
const Token FOR_KEYWORD = Token{Token_type::KEYWORD, "for"};
const Token IF_KEYWORD = Token{Token_type::KEYWORD, "if"};
const Token WHILE_KEYWORD = Token{Token_type::KEYWORD, "while"};
const Token IN_KEYWORD = Token{Token_type::KEYWORD, "in"};
const Token BY_KEYWORD = Token{Token_type::KEYWORD, "by"};

const Token OPEN_PAREN = Token{Token_type::SYMBOL, "("};
const Token OPEN_BRACKET = Token{Token_type::SYMBOL, "["};
const Token OPEN_BRACE = Token{Token_type::SYMBOL, "{"};

const Token CLOSING_PAREN = Token{Token_type::SYMBOL, ")"};
const Token CLOSING_BRACKET = Token{Token_type::SYMBOL, "]"};
const Token CLOSING_BRACE = Token{Token_type::SYMBOL, "}"};

const Token GETTER_TOKEN = Token{Token_type::SYMBOL, "."};
const Token CAST_TOKEN = Token{Token_type::SYMBOL, "_"};



string Function_type::get_type_id() const
{
    ostringstream oss{"fn("};
    bool first = true;
    for (auto& type : in_parameters) {
        if (!first) { oss << ","; first = true; }
        oss << type->get_type_id();
    }
    oss << ")";
    if (!out_parameters.empty()) {
        oss << "->";
        bool first = true;
        for (auto& type : out_parameters) {
            if (!first) { oss << ","; first = true; }
            oss << type->get_type_id();
        }
    }
    return oss.str();
}


string Struct_type::get_type_id() const
{
    ostringstream oss{"struct{"};
    bool first = true;
    for (auto& member : members) {
        ASSERT(member->identifier_token != nullptr);
        ASSERT(member->type != nullptr);
        if (!first) { oss << ","; first = true; }
        oss << member->identifier_token->token << ":" << member->type->get_type_id();
    }
    oss << "}";
    return oss.str();
}








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


bool is_infix_operator(const Token& t)
{
    if (t.type != Token_type::SYMBOL) return false;

    if (t.token == "==") return true;
    if (t.token == ">=") return true;
    if (t.token == "<=") return true;
    if (t.token == "<") return true;
    if (t.token == ">") return true;
    if (t.token == "&&") return true; // TODO: find some other AND operator
    if (t.token == "||") return true; // TODO: find some other OR operator
    if (t.token == "+") return true;
    if (t.token == "-") return true;
    if (t.token == "*") return true;
    if (t.token == "/") return true;
    if (t.token == "%") return true;

    return false;
}

// higher priority -> higher returned number
int get_infix_priority(const Token& t)
{
    ASSERT(is_infix_operator(t));

    // Low priority
    if (t.token == "==") return 0;
    if (t.token == ">=") return 0;
    if (t.token == "<=") return 0;
    if (t.token == "<") return 0;
    if (t.token == ">") return 0;

    if (t.token == "&&") return 1; // TODO: find some other AND operator
    if (t.token == "||") return 1; // TODO: find some other OR operator

    if (t.token == "+") return 2;
    if (t.token == "-") return 2;

    if (t.token == "*") return 3;
    if (t.token == "/") return 3;
    if (t.token == "%") return 3;
    // high priority
}



bool is_closing_symbol(const Token& t)
{
    if (t.type != Token_type::SYMBOL) return false;

    if (t.token == ")") return true;
    if (t.token == "]") return true;
    if (t.token == "}") return true;

    return false;
}





// start must point at the opening thing
// returns pointers to the closing thing

Token const * read_paren(Token const * it);
Token const * read_bracket(Token const * it);
Token const * read_brace(Token const * it);

Token const * read_token_range_recursive(Token const * it, const string& expected_closing_token, const string& range_name)
{
    Token const* start = it;
    while(true) {
        if ((++it)->type == Token_type::EOF) {
            log_error("Missing \""+expected_closing_token+"\" at end of file",it->context);
            return nullptr;
        }
        if (it->type == Token_type::SYMBOL) {
            if (is_closing_symbol(*it)) {
                if (it->token != expected_closing_token) {
                    log_error("Mismatched "+range_name+": expected \""+expected_closing_token+"\" before \""+it->token+"\"",it->context);
                    add_note("In "+range_name+" that started here: ",start->context);
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


bool read_evaluated_variable(Token const*& it, unique_ptr<Evaluated_variable>& variable, Scope* scope);
bool read_evaluated_value(Token const*& it, unique_ptr<Evaluated_value>& value, Scope* scope);


// variable:
//      in: the function identifier
//      out: the function itself
bool read_function_call(Token const*& it, unique_ptr<Evaluated_variable>& variable, Scope* scope)
{
    ASSERT(it != nullptr && *it == OPEN_PAREN);
    unique_ptr<Function_call> fc{new Function_call()};
    fc->function_identifier = move(variable);

    if (*(++it) != CLOSING_PAREN) { // if it's not an empty variable list
        while (true) {
            if (it->type == Token_type::EOF) {
                log_error("Unexpected end of file in function call: expected argument",it->context);
                return true;
            }
            unique_ptr<Evaluated_value> argument;
            if (read_evaluated_value(it,argument,scope)) return true; // error

            fc->arguments.push_back(move(argument));

            if (*it == CLOSING_PAREN) break; // ok
            if (*it != COMMA) {
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
bool read_getter(Token const*& it, unique_ptr<Evaluated_variable>& variable, Scope* scope)
{
    ASSERT(it != nullptr && *it == GETTER_TOKEN);
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
bool read_cast(Token const*& it, unique_ptr<Evaluated_value>& variable, Scope* scope)
{
    ASSERT(it != nullptr && *it == CAST_TOKEN);
    unique_ptr<Cast> cast{new Cast()};
    cast->casted_value = move(variable);

    if ((++it)->type == Token_type::IDENTIFIER) {
        if (read_evaluated_variable(it,cast->casted_type,scope)) return true; // error
        variable = move(cast);
        return false; // ok!
    } else {
        log_error("Expected type identifier after cast token \"_\"",it->context);
        return true;
    }
}
bool read_cast(Token const*& it, unique_ptr<Evaluated_variable>& variable, Scope* scope)
{
    unique_ptr<Evaluated_value> value = move(variable);
    if (read_cast(it,value,scope)) return true; // error
    ASSERT(dynamic_cast<Evaluated_variable*>(value.get()) != nullptr);
    variable.reset(static_cast<Evaluated_variable*>(value.release()));
}


// variable:
//      in: the array identifier
//      out: the lookup itself
bool read_array_lookup(Token const*& it, unique_ptr<Evaluated_variable>& variable, Scope* scope)
{
    ASSERT(it != nullptr && *it == OPEN_BRACKET);
    unique_ptr<Array_lookup> al{new Array_lookup()};
    al->array_identifier = move(variable);

    if (read_evaluated_value((++it),al->position,scope)) return true; // error
    if (*it != CLOSING_BRACKET) {
        log_error("Missing \"]\" at the end of array lookup",it->context);
        return true;
    }
    ++it; // go past the "]" token
    variable = move(al);
    return false;
}



bool read_call_chain(Token const*& it, unique_ptr<Evaluated_variable>& variable, Scope* scope)
{
    while(++it) {
        if (it->type != Token_type::SYMBOL) return false; // end of evaluated_variable
        if (it->token == "(") {
            // read function call
            if (read_function_call(it,variable,scope)) return true; // error
            ASSERT(*(it-1) == CLOSING_PAREN); // should be the end of the function call
        } else if (it->token == ".") {
            // read getter
            if (read_getter(it,variable,scope)) return true;
        } else if (it->token == "_") {
            // read cast
            if (read_cast(it,variable,scope)) return true;
        } else if (it->token == "[") {
            // read array lookup
            if (read_array_lookup(it,variable,scope)) return true;
            ASSERT((it-1)->type == Token_type::SYMBOL && (it-1)->token == "]"); // should be the end of the array lookup
        }
    }
}



bool read_evaluated_variable(Token const*& it, unique_ptr<Evaluated_variable>& variable, Scope* scope)
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
    read_call_chain(it,variable,scope);
    return false;
}



// maybe: use this in read_function_call()
bool read_value_list(Token const*& it, unique_ptr<Evaluated_value>& value, Scope* scope)
{
    ASSERT(it != nullptr && it->type == Token_type::SYMBOL && it->token == "(");
    it++; // to past the "(" token

    unique_ptr<Value_list> vl{new Value_list()};

    if (it->type == Token_type::SYMBOL && it->token == ")") {
        // empty list
        return false;
    }

    while(true) {
        unique_ptr<Evaluated_value> v;
        if (read_evaluated_value(it,v,scope)) return true;
        ASSERT(v != nullptr);
        vl->values.push_back(move(v));
        if (it->type == Token_type::SYMBOL && it->token == ")") {
            it++; // go past the ")" token
            value = move(vl);
            return false; // ok
        }
        if (it->type != Token_type::SYMBOL || it->token != ",") {
            log_error("Unexpected token in value list: expected \",\" between values",it->context);
            return true; // error
        }
        it++; // go past the "," token
    }
}






bool read_infix_op(Token const*& it, unique_ptr<Evaluated_value>& value, Scope* scope)
{
    ASSERT(it != nullptr && is_infix_operator(*it));
    Token const * op_token = it;

    unique_ptr<Evaluated_value> rhs;
    if (read_evaluated_value(it,rhs,scope)) return true;

    if (!is_infix_operator(*it)) {
        unique_ptr<Infix_op> infix{new Infix_op()};
        infix->lhs = move(value);
        infix->op_token = op_token;
        infix->rhs = move(rhs);
        return false;
    }
    else {
        // chained infix op
        Token const * op_token2 = it;
        unique_ptr<Evaluated_value> rhs2;
        if (read_evaluated_value(it,rhs2,scope)) return true;

        unique_ptr<Infix_op> lhs_infix{new Infix_op()};
        unique_ptr<Infix_op> rhs_infix{new Infix_op()};

        lhs_infix->lhs = move(value);
        lhs_infix->op_token = op_token;

        rhs_infix->op_token = op_token2;
        rhs_infix->rhs = move(rhs2);

        // check priority
        if (get_infix_priority(*op_token) < get_infix_priority(*op_token2)) {
            // lhs + (rhs * rhs2)
            rhs_infix->lhs = move(rhs);
            lhs_infix->rhs = move(rhs_infix);
            value = move(lhs_infix);
        } else {
            // (lhs * rhs) + rhs2
            lhs_infix->rhs = move(rhs);
            rhs_infix->lhs = move(lhs_infix);
            value = move(rhs_infix);
        }
        return false;
    }
}





bool read_evaluated_value(Token const*& it, unique_ptr<Evaluated_value>& value, Scope* scope)
{
    // read value. Except for Evaluated_variables, this can also be literals: string, integer, float, bool, array

    ASSERT(it != nullptr);
    value = nullptr;

    if (it->type == Token_type::IDENTIFIER) {
        unique_ptr<Evaluated_variable> variable;
        if (read_evaluated_variable(it,variable,scope)) return true; // error
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
        if (read_value_list(it,value,scope)) return true;
        ASSERT(value != nullptr);
        ASSERT((it-1)->type == Token_type::SYMBOL && (it-1)->token == ")");
    }

    // if followed by "_" -> cast
    // That transforms the value into a variable -> can read call chain:
    if (it->type == Token_type::SYMBOL && it->token == "_") {
        if (read_cast(it,value,scope)) return true;
        ASSERT(dynamic_cast<Evaluated_variable*>(value.get()) != nullptr);
        unique_ptr<Evaluated_variable> variable{static_cast<Evaluated_variable*>(value.release())};
        read_call_chain(it,variable,scope);
        value = move(variable);
    }

    // if followd by infix operator -> read infix op
    if (is_infix_operator(*it)) {
        read_infix_op(it,value,scope);
    }
    // that should take care of all chained infix operators as well.
    ASSERT(!is_infix_operator(*it));

    return false;
}









/*

function definitions:

// tills vidare: endast simpel syntax tillåten:
fn(a:int, b:float); // ok
fn(a:int, b:float=2); // ok, men senare @default_values

type specificaion:
fn(int,float); // endast typer tillåtna, inga variabelnamn.
fn(fn()); // ok




// KANSKE:
fn(a,b : int,float);
fn(a:int = 2, b:=2.0);
fn(a:int, b,c:float,float);
fn((a=b):int, c,d:float) // error, men vilket error? För complext. inte ok!

*/





// comma separated list with either pure identifiers, or a parenthesis of "="-separated identifiers
bool read_declaration_lhs_part(Token const* it, vector<Token const*>& variable_tokens)
{
    ASSERT(it != nullptr);
    if (it->type == Token_type::IDENTIFIER) {
        variable_tokens.push_back(it);
        return false;
    }

    if (it->type == Token_type::SYMBOL && it->token == "(") {
        while (true) {
            ++it; // go past the "(" or "," token
            if (it->type != Token_type::IDENTIFIER) {
                log_error("Unexpected token in lhs of declaration. Expected identifier.",it->context);
                return true;
            }
            variable_tokens.push_back(it);
            if ((++it)->type == Token_type::SYMBOL && it->token == ")") {
                ++it; // go past the ")" token
                return false; // ok
            }

            if (it->type != Token_type::SYMBOL || it->token != "=") {
                log_error("Unexpected token in lhs of declaration. Expected \"=\" between identifiers",it->context);
                return true;
            }
        }
    }
}

bool read_declaration_lhs(Token const* it, vector<vector<Token const*>>& variable_tokens)
{
    ASSERT(it != nullptr);
    while(true) {
        vector<Token const*> v;
        if (read_declaration_lhs_part(it,v)) return true;
        ASSERT(!v.empty());
        variable_tokens.push_back(v);
        if (it->type != Token_type::SYMBOL || it->token != ",") return false;
    }
}




// TODO: if the identifier already exist in the local scope -> error (Except for @overloading)
// TODO: identifiers can be used before they are declared.
//      In that case, they should be added without a declaration context. (?)
bool add_identifiers_to_scope(const Declaration& declaration, Scope* scope)
{
    bool errors = false;
    for (const auto& lhs_part : declaration.variable_name_tokens) {
        for (Token const* token : lhs_part) {

            bool found = false;
            for (auto& id : scope->identifiers) {
                if (id->identifier_token->token == token->token) {
                    log_error("Multiple declarations of identifier \""+token->token+"\"",token->context);
                    add_note("Previously declared here",id->identifier_token->context);
                    errors = true;
                    found = true;
                }
            }
            if (!found) {
                unique_ptr<Typed_identifier> id{new Typed_identifier()};
                id->identifier_token = token;
                // type is nullptr (unknown) for now.
                scope->identifiers.push_back(move(id));
            }
        }
    }
    return errors;
}




/*
A type can be:

    A single identifier
    A function signature
    A struct


defined_int : type = int;
a : defined_int = 2;

defined_struct : type = struct{ a : int; }; // Allting måste ha en typ. Inga default values än. (todo)
b : defined_struct;
b.a = 2;

defined_fn_type := fn(int)->int;
foo : fn(int)->int = fn(a : int) -> int { return 2; }; // rhs MÅSTE ha identifiers OCH body.
foo : defined_fn_type = fn(a : int) -> int { return 2; };

*/
bool read_type(Token const * it, unique_ptr<Type_info>& info);



bool read_function_type(Token const * it, unique_ptr<Function_type>& ft)
{
    ASSERT(it != nullptr && it->type == Token_type::KEYWORD && it->token == "fn");

    ft.reset(new Function_type());

    // function identifier: fn(int,int)->int
    if ((++it)->type != Token_type::SYMBOL || it->token != "(") {
        log_error("Expected \"(\" after \"fn\" keyword",it->context);
        return true;
    }
    ++it; // go past the "(" token
    if (it->type != Token_type::SYMBOL || it->token != ")") {
        while(true) {
            unique_ptr<Type_info> parameter_type;
            if (read_type(it,parameter_type)) return true;
            ASSERT(parameter_type != nullptr);
            ft->in_parameters.push_back(move(parameter_type));

            if (it->type == Token_type::SYMBOL && it->token == ")") break; // ok
            if (it->type != Token_type::SYMBOL || it->token != ",") {
                log_error("Expected \",\" between types in function type declaration",it->context);
                return true;
            }
            it++; // go past the "," token
        }
    }
    ASSERT(it->type == Token_type::SYMBOL && it->token == ")");
    ++it; // go past the ")" token

    if (it->type == Token_type::SYMBOL && it->token == "->") {
        // read out parameter list
        while(true) {
            unique_ptr<Type_info> parameter_type;
            if (read_type(it,parameter_type)) return true;
            ASSERT(parameter_type != nullptr);
            ft->out_parameters.push_back(move(parameter_type));

            if (it->type != Token_type::SYMBOL || it->token != ",") break; // ok
            it++; // go past the "," token
        }
    }

    return false; // OK!
}






bool read_type_list(Token const * it, vector<unique_ptr<Type_info>>& v)
{
    ASSERT(it != nullptr);

    while(true) {
        unique_ptr<Type_info> type;
        if (read_type(it,type)) return true;
        ASSERT(type != nullptr);
        v.push_back(move(type));
        if (it->type != Token_type::SYMBOL || it->token != ",") return false;
        it++; // go past the "," token
    }
}


/*
vector<unique_ptr<Typed_identifier>> match_types(const vector<unique_ptr<Identifier>>& ids, const vector<unique_ptr<Type_info>>& types)
{
    if (types.size() == 0) {
        // only unresolved types
        for (auto& id : ids) {
            unique_ptr<Typed_identifier> tid {new Typed_identifier()};
            tid->identifier_token = id->identifier_token;
            result.push_back(move(tid));
        }
        return result;
    }

    ASSERT(types.size() == id.size()); // this should give an error earlier
    // log_error(string("Type mismatch: expected ")+id.size()+" types but found only "+types.size(), it->context);

    for (int i = 0; i < types.size(); ++i) {
        unique_ptr<Typed_identifier> tid {new Typed_identifier()};
        tid->identifier_token = ids[i]->identifier_token;
        tid->type = types[i].get();
        result.push_back(move(tid));
    }
    return result;
}
*/



/*
struct {
    name : type;
    n1, n2 : t1, t2;
}

// TODO: default values
// TODO: using statements

*/
bool read_struct_type(Token const * it, unique_ptr<Struct_type>& st)
{
    ASSERT(it != nullptr && it->type == Token_type::KEYWORD && it->token == "struct");

    st.reset(new Struct_type());

    if ((++it)->type != Token_type::SYMBOL || it->token != "{") {
        log_error("Expected \"}\" after \"struct\"",it->context);
        return true;
    }
    ++it; // go past the "{" token
    if (it->type == Token_type::SYMBOL && it->token == "}") return false; // ok

    do {
        vector<unique_ptr<Typed_identifier>> tid_to_add;

        while (true) {
            if (it->type != Token_type::IDENTIFIER) {
                log_error("Unexpected token in struct: expected identifier",it->context);
                return true;
            }
            unique_ptr<Typed_identifier> tid{};
            tid->identifier_token = it;
            tid_to_add.push_back(move(tid));
            if (it->type == Token_type::SYMBOL && it->token == ":") break;
            if (it->type != Token_type::SYMBOL || it->token != ",") {
                log_error("Unexpected token in struct: expected \",\" between identifiers",it->context);
                return true;
            }
            it++; // go past the "," token
        }
        ASSERT(it->type == Token_type::SYMBOL && it->token == ":");
        it++; // go past the ":" token

        for (auto& tid : tid_to_add) {
            if (read_type(it,tid->type)) return true; // this handles the "too few types" issue
            st->members.push_back(move(tid));
        }

        if (it->type != Token_type::SYMBOL || it->token != ";") {
            // this handles the "too many types" issue
            log_error("Unexpected token in struct: expected \";\" after member declaration",it->context);
            return true;
        }
        ++it; // go past the ";" token
    } while (it->type != Token_type::SYMBOL || it->token != "}");

    if (it->type != Token_type::SYMBOL || it->token != "}") {
        log_error("Missing \"}\" at the end of struct",it->context);
        return true;
    }
    it++; // go past the "}" token
    return false; // ok
}




bool read_type(Token const * it, unique_ptr<Type_info>& info)
{
    ASSERT(it!=nullptr);

    if (it->type == Token_type::IDENTIFIER) {
        // type identifier - just one token
        unique_ptr<Unresolved_type> ut{new Unresolved_type()};
        ut->identifier_token = it;
        info = move(ut);
        ++it; // go past the identifier token
        return false;
    }

    if (it->type == Token_type::KEYWORD && it->token == "fn") {
        unique_ptr<Function_type> ft;
        if (read_function_type(it,ft)) return true;
        ASSERT(ft != nullptr);
        info = move(ft);
        return false;
    }

    if (it->type == Token_type::KEYWORD && it->token == "struct") {
        unique_ptr<Struct_type> st;
        if (read_struct_type(it,st)) return true;
        ASSERT(st != nullptr);
        info = move(st);
        return false;
    }

    log_error("Unexpected token: Expected type",it->context);
    return true;
}






// returns true if errors
// returns false if syntax seems good, even if not able to deduce types yet. In that case, dependencies are added.
bool read_declaration(Token const* it, Token const * type_token, Token const * assignment_token, Token const * end_token, Scope* scope, bool force_static)
{
    ASSERT(type_token->type == Token_type::SYMBOL);
    ASSERT(type_token->token == ":");
    // Read declaration

    unique_ptr<Declaration> declaration{new Declaration()};
    bool errors = false; // we want to continue to rhs in case we find a scope -> don't break on lhs

    // read lhs
    if (read_declaration_lhs(it,declaration->variable_name_tokens)) errors = true;

    // Add the declared identifiers to the scope (even if we get errors later)
    if (add_identifiers_to_scope(*declaration,scope)) errors = true;

    // if (it->type != Token_type::SYMBOL || it->token != ":") {
    if (it != type_token) {
        log_error("Unexpected token in declaration: expected \":\" directly after lhs. ", it->context);
        add_note("\":\" found here",type_token->context);
        errors = true;
        it = type_token;
    }

    // a : int;
    // a : int = 2; // type identifier list
    // a := b;
    // a : ; // error
    // a := ; // error

    bool found_types = false;
    bool found_rhs = false;

    // it = ":"
    if ((++it)->type != Token_type::SYMBOL || it->token != "=") {
        // TODO: read type identifier list
        found_types = true;
    }

    // it == "=" or ";"
    if (it->type == Token_type::SYMBOL && it->token == "=") {
        // TODO: read rhs. bool force_static is useful here.
        found_rhs = true;
    }

    if (!found_types && !found_rhs) {
        log_error("Missing tokens after declaration: must specify either type or assignment",it->context);
        errors = true;
    }

    if (it->type != Token_type::SYMBOL || it->token == ";") {
        log_error("Missing \";\" after declaration", it->context);
        add_note("\";\" found here",end_token->context);
        errors = true;
    } else {
        ++it; // go past the ";" token
    }

    return false;


    /*
    // OLD CODE BELOW

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
    */
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
            if (read_evaluated_value(it,value,scope)) {
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


/*

dynamic context:

statements can be anonymous scopes:

b : int;
{ // pulled in parent scope, we can use and change b as usual
    a : int = 2;
    b += a;
}

can have capture group

b : int = 2;
[b]{ // capture by value
   a := b;
   printf("%",a); // 2
   b = 5; // only local
}
printf("%",b); // 2


can be function

b : int = 2;
fn(int x){ // pulls in the parent STATIC scope. That means that b is unusable.
    // a := b; // error: b i not defined
    printf("%",x);
}(b); // call the function which prints 2

function with capture group

b : int = 2;
fn(int x)[b]{ // pulls in the value of b at the point of function creation
    b = 5;
    printf("%,%",x,b); // 2,5
}(b);
printf("%",b); // still 2

*/









unique_ptr<Scope> parse_tokens(const vector<Token>& tokens)
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

unique_ptr<Scope> parse_file(const string& file)
{
    return parse_tokens(get_tokens_from_file(file));
}

unique_ptr<Scope> parse_string(const string& string)
{
    return parse_tokens(get_tokens_from_string(string));
}
