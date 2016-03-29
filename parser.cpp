#include "parser.h"
#include "error_handler.h"
#include "lexer.h"
#include <sstream>

using namespace std;

/*

Return bool: true if errors, false if ok
Take Token const*& it as first argument
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
const Token RETURN_KEYWORD = Token{Token_type::KEYWORD, "return"};
const Token STRUCT_KEYWORD = Token{Token_type::KEYWORD, "struct"};
const Token FOR_KEYWORD = Token{Token_type::KEYWORD, "for"};
const Token IF_KEYWORD = Token{Token_type::KEYWORD, "if"};
const Token ELSE_KEYWORD = Token{Token_type::KEYWORD, "else"};
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

const Token ASSIGNMENT_TOKEN = Token{Token_type::SYMBOL, "="};
const Token RIGHT_ARROW = Token{Token_type::SYMBOL, "->"};
const Token LEFT_ARROW = Token{Token_type::SYMBOL, "<-"};

const Token DOUBLE_EQUALS = Token{Token_type::SYMBOL, "=="};

const Token DUMP_TOKEN = Token{Token_type::SYMBOL, "_"};

string Function_type::get_type_id() const
{
    ostringstream oss{};
    oss << "fn(";
    bool first = true;
    for (auto& type : in_parameters) {
        if (!first) oss << ",";
        first = false;
        oss << type->get_type_id();
    }
    oss << ")";
    if (!out_parameters.empty()) {
        oss << "->";
        bool first = true;
        for (auto& type : out_parameters) {
            if (!first) oss << ",";
            first = false;
            oss << type->get_type_id();
        }
    }
    return oss.str();
}


string Struct_type::get_type_id() const
{
    ostringstream oss{};
    oss << "struct{";
    bool first = true;
    for (auto& member : members) {
        ASSERT(member->identifier_token != nullptr);
        ASSERT(member->type != nullptr);
        if (!first) oss << ",";
        first = false;
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

    if (*it == IF_KEYWORD || *it == FOR_KEYWORD || *it == WHILE_KEYWORD) return false;

    while(true) {
        if (it->type == Token_type::EOF || is_closing_symbol(*it)) {
            log_error("Missing \";\" after statement",it->context);
            add_note("Expected \";\" before \""+it->token+"\"");
            end_token = it;
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


bool read_evaluated_variable(Token const*& it, unique_ptr<Evaluated_variable>& variable, Scope* scope, bool force_static);
bool read_evaluated_value(Token const*& it, unique_ptr<Evaluated_value>& value, Scope* scope, bool force_static);


// variable:
//      in: the function identifier
//      out: the function itself
bool read_function_call(Token const*& it, unique_ptr<Evaluated_value>& value, Scope* scope)
{
    ASSERT(it != nullptr && *it == OPEN_PAREN);
    unique_ptr<Function_call> fc{new Function_call()};
    fc->function_identifier = move(value);

    if (*(++it) != CLOSING_PAREN) { // if it's not an empty variable list
        while (true) {
            if (it->type == Token_type::EOF) {
                log_error("Unexpected end of file in function call: expected argument",it->context);
                return true;
            }
            unique_ptr<Evaluated_value> argument;
            if (read_evaluated_value(it,argument,scope,false)) return true; // error

            fc->arguments.push_back(move(argument));

            if (*it == CLOSING_PAREN) break; // ok
            if (*it != COMMA) {
                log_error("Unexpected token in function call argument list. Expected \",\" between arguments",it->context);
                return true;
            }
            ++it; // go past the "," token
        }
    }

    ASSERT(*it == CLOSING_PAREN);

    ++it; // go past the ")" token
    value = move(fc);
    return false; // ok!
}



// variable:
//      in: the struct identifier
//      out: the getter itself
bool read_getter(Token const*& it, unique_ptr<Evaluated_value>& value, Scope* scope)
{
    ASSERT(it != nullptr && *it == GETTER_TOKEN);
    unique_ptr<Getter> g{new Getter()};
    g->struct_identifier = move(value);

    if ((++it)->type == Token_type::IDENTIFIER) {
        g->data_identifier_token = it;
        value = move(g);
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
bool read_cast(Token const*& it, unique_ptr<Evaluated_value>& value, Scope* scope)
{
    ASSERT(it != nullptr && *it == CAST_TOKEN);
    unique_ptr<Cast> cast{new Cast()};
    cast->casted_value = move(value);

    if ((++it)->type == Token_type::IDENTIFIER) {
        if (read_evaluated_variable(it,cast->casted_type,scope,false)) return true; // error
        value = move(cast);
        return false; // ok!
    } else {
        log_error("Expected type identifier after cast token \"_\"",it->context);
        return true;
    }
}


// variable:
//      in: the array identifier
//      out: the lookup itself
bool read_array_lookup(Token const*& it, unique_ptr<Evaluated_value>& value, Scope* scope, bool force_static)
{
    ASSERT(it != nullptr && *it == OPEN_BRACKET);
    unique_ptr<Array_lookup> al{new Array_lookup()};
    al->array_identifier = move(value);

    if (read_evaluated_value((++it),al->position,scope,force_static)) return true; // error
    if (*it != CLOSING_BRACKET) {
        log_error("Missing \"]\" at the end of array lookup",it->context);
        return true;
    }
    ++it; // go past the "]" token
    value = move(al);
    return false;
}




bool read_call_chain(Token const*& it, unique_ptr<Evaluated_value>& value, Scope* scope, bool force_static)
{
    while(true) {
        if (it->type != Token_type::SYMBOL) return false; // end

        if (it->token == "(") {
            if (read_function_call(it,value,scope)) return true; // error

        } else if (it->token == ".") {
            if (read_getter(it,value,scope)) return true;

        } else if (it->token == "_") {
            if (read_cast(it,value,scope)) return true;

        } else if (it->token == "[") {
            if (read_array_lookup(it,value,scope,force_static)) return true;

        } else return false; // unknown symbol -> end
    }
}
bool read_call_chain(Token const*& it, unique_ptr<Evaluated_variable>& variable, Scope* scope, bool force_static)
{
    unique_ptr<Evaluated_value> value = move(variable);
    if (read_call_chain(it,value,scope,force_static)) return true; // error
    ASSERT(dynamic_cast<Evaluated_variable*>(value.get()) != nullptr);
    variable.reset(static_cast<Evaluated_variable*>(value.release()));
    return false;
}



bool read_evaluated_variable(Token const*& it, unique_ptr<Evaluated_variable>& variable, Scope* scope, bool force_static)
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
    it++; // go past the identifier token

    if (read_call_chain(it,variable,scope,force_static)) return true;
    return false;
}



// maybe: use this in read_function_call()
bool read_value_list(Token const*& it, unique_ptr<Evaluated_value>& value, Scope* scope)
{
    ASSERT(it != nullptr && *it == OPEN_PAREN);
    it++; // to past the "(" token

    unique_ptr<Value_list> vl{new Value_list()};

    if (*it == CLOSING_PAREN) {
        // empty list
        return false;
    }

    while(true) {
        unique_ptr<Evaluated_value> v;
        if (read_evaluated_value(it,v,scope,false)) return true;
        ASSERT(v != nullptr);
        vl->values.push_back(move(v));
        if (*it == CLOSING_PAREN) {
            it++; // go past the ")" token
            value = move(vl);
            return false; // ok
        }
        if (*it != COMMA) {
            log_error("Unexpected token in value list: expected \",\" between values",it->context);
            return true; // error
        }
        it++; // go past the "," token
    }
}



bool read_type(Token const *& it, shared_ptr<Type_info>& info);
bool read_dynamic_scope(Token const *& it, unique_ptr<Dynamic_scope>& scope, Scope* parent_scope);

// a function can either be a function type declaration, or a defined function

// function declaration: a := fn(int,float,fn(int));
// function definition: a := fn(a:int,b:float,c:fn(int)) {}


bool read_function(Token const*& it, unique_ptr<Evaluated_value>& function, Scope* scope)
{
    ASSERT(it != nullptr && *it == FUNCTION_KEYWORD);

    if (*(++it) != OPEN_PAREN) {
        log_error("Missing \"(\" after \"fn\"",it->context);
        return true;
    }

    unique_ptr<Function> fn{new Function()};
    unique_ptr<Function_type> fn_type{new Function_type()};

    bool is_definition; // false if unknown
    bool is_declaration; // false if unknown

    if (*(++it) != CLOSING_PAREN) {
        // read in parameters
        bool named = it->type == Token_type::IDENTIFIER && *(it+1) == COLON;
        is_declaration = !named;
        is_definition = named;

        while (true) {
            if (named) {
                if (it->type != Token_type::IDENTIFIER) {
                    log_error("Unexpected token in function parameter list: expected identifier",it->context);
                    ostringstream oss;
                    oss << "found token \"" << it->token << "\" of type " << it->type;
                    add_note(oss.str());
                    return true;
                }
                fn->in_parameter_name_tokens.push_back(it);

                if (*(++it) != COLON) {
                    log_error("Missing \":\" after identifier in function parameter list",it->context);
                    return true;
                }
                ++it; // go past the ":" token
            }
            shared_ptr<Type_info> type;
            if (read_type(it,type)) return true;
            ASSERT(type != nullptr);
            fn_type->in_parameters.push_back(type);

            if (*it == CLOSING_PAREN) break; // ok
            if (*it != COMMA) {
                log_error("Missing \",\" between parameters",it->context);
                return true;
            }
            it++; // go past the "," token
        }
    }
    ASSERT(*it == CLOSING_PAREN);
    it++; // go past the ")" token

    if (*it == RIGHT_ARROW) {
        // read return value list
        it++; // go past the "->" token

        while (true) {
            bool named = it->type == Token_type::IDENTIFIER && *(it+1) == COLON;
            if (named && is_declaration) {
                log_error("Named return values not allowed in function declaration",it->context);
                return true;
            }
            if (named) {
                fn->out_parameter_name_tokens.push_back(it);
                ASSERT (*(++it) == COLON);
                ++it; // go past the ":" token
            } else fn->out_parameter_name_tokens.push_back(&DUMP_TOKEN);
            shared_ptr<Type_info> type;
            if (read_type(it,type)) return true;
            ASSERT(type != nullptr);
            fn_type->out_parameters.push_back(type);

            if (*it != COMMA) break;
        }
    }

    if (is_declaration) {
        if (*it == OPEN_BRACE) {
            log_error("Function body not allowed after declaration",it->context);
            return true;
        }
        function = move(fn_type);
        return false;
    }

    if (*it != OPEN_BRACE) {
        if (is_definition) {
            log_error("Missing function body after definition",it->context);
            // TODO: accept capture group
            return true;
        }
        function = move(fn_type);
        return false;
    }
    if (read_dynamic_scope(it,fn->body,scope)) return true;
    fn->type = move(fn_type);
    function = move(fn);
    return false;
}




bool read_infix_op(Token const*& it, unique_ptr<Evaluated_value>& value, Scope* scope, bool force_static)
{
    ASSERT(it != nullptr && is_infix_operator(*it));
    Token const * op_token = it;
    it++; // go past the infix token

    unique_ptr<Evaluated_value> rhs;
    if (read_evaluated_value(it,rhs,scope,force_static)) return true;

    if (!is_infix_operator(*it)) {
        unique_ptr<Infix_op> infix{new Infix_op()};
        infix->lhs = move(value);
        infix->op_token = op_token;
        infix->rhs = move(rhs);
        value = move(infix);
        return false;
    }
    else {
        // chained infix op
        Token const * op_token2 = it;
        unique_ptr<Evaluated_value> rhs2;
        if (read_evaluated_value(it,rhs2,scope,force_static)) return true;

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




bool read_struct_type(Token const *& it, unique_ptr<Struct_type>& st);

bool read_evaluated_value(Token const*& it, unique_ptr<Evaluated_value>& value, Scope* scope, bool force_static)
{
    // read value. Except for Evaluated_variables, this can also be literals: string, integer, float, bool, array

    ASSERT(it != nullptr);
    value = nullptr;

    if (it->type == Token_type::IDENTIFIER) {
        unique_ptr<Evaluated_variable> variable;
        if (read_evaluated_variable(it,variable,scope,force_static)) return true; // error
        value = move(variable);
        ASSERT(value != nullptr);

    } else if (it->type == Token_type::STRING ||
        it->type == Token_type::INTEGER ||
        it->type == Token_type::FLOAT ||
        it->type == Token_type::BOOL)
    {
        // literal
        unique_ptr<Literal> literal{new Literal()};
        literal->literal_token = it;
        value = move(literal);
        ++it;

    } else if (*it == OPEN_PAREN) {
        // Value_list
        if (read_value_list(it,value,scope)) return true;
        ASSERT(value != nullptr);
        ASSERT(*(it-1) == CLOSING_PAREN);

    } else if (*it == OPEN_BRACKET) {
        // capture group before scope
        log_error("Reading scopes with capture group not yet implemented (read_evaluated_value)",it->context);
        return true;

    } else if (*it == OPEN_BRACE) {
        // scope or array literal
        log_error("Reading scopes not yet implemented (read_evaluated_value)",it->context);
        log_error("Reading array literals not yet implemented (read_evaluated_value)",it->context);
        return true;

    } else if (*it == FUNCTION_KEYWORD) {
        // function definition or declaration
        if (read_function(it,value,scope)) return true;
        ASSERT(value != nullptr);

    } else if (*it == STRUCT_KEYWORD) {
        // struct
        unique_ptr<Struct_type> st;
        if (read_struct_type(it,st)) return true;
        ASSERT(st != nullptr);
        value = move(st);

    } else {
        log_error("Unexpected token while reading value: a value cannot start with the token \""+it->token+"\"",it->context);
        return true;
    }

    // read chain of casts, function calls, getters, array lookups if applicable.
    // If lhs doesn't have the operator -> error in type checker
    if (read_call_chain(it,value,scope,force_static)) return true;
    ASSERT(value != nullptr);

    // if followd by infix operator -> read infix op
    if (is_infix_operator(*it)) {
        if (read_infix_op(it,value,scope,force_static)) return true;
        ASSERT(value != nullptr);
    }
    // that should take care of all chained infix operators as well.
    ASSERT(!is_infix_operator(*it));

    return false;
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



bool read_function_type(Token const *& it, unique_ptr<Function_type>& ft)
{
    ASSERT(it != nullptr && *it == FUNCTION_KEYWORD);

    ft.reset(new Function_type());

    // function identifier: fn(int,int)->int
    if (*(++it) == OPEN_PAREN) {
        log_error("Expected \"(\" after \"fn\" keyword",it->context);
        return true;
    }
    ++it; // go past the "(" token
    if (*it != CLOSING_PAREN) {
        while(true) {
            shared_ptr<Type_info> parameter_type;
            if (read_type(it,parameter_type)) return true;
            ASSERT(parameter_type != nullptr);
            ft->in_parameters.push_back(move(parameter_type));

            if (*it == CLOSING_PAREN) break; // ok
            if (*it != COMMA) {
                log_error("Expected \",\" between types in function type declaration",it->context);
                return true;
            }
            it++; // go past the "," token
        }
    }
    ASSERT(*it == CLOSING_PAREN);
    ++it; // go past the ")" token

    if (*it == RIGHT_ARROW) {
        // read out parameter list
        while(true) {
            shared_ptr<Type_info> parameter_type;
            if (read_type(it,parameter_type)) return true;
            ASSERT(parameter_type != nullptr);
            ft->out_parameters.push_back(move(parameter_type));

            if (*it != COMMA) break; // ok
            it++; // go past the "," token
        }
    }

    return false; // OK!
}






bool read_type_list(Token const *& it, vector<shared_ptr<Type_info>>& v)
{
    ASSERT(it != nullptr);

    while(true) {
        shared_ptr<Type_info> type;
        if (read_type(it,type)) return true;
        ASSERT(type != nullptr);
        v.push_back(move(type));
        if (*it != COMMA) return false;
        it++; // go past the "," token
    }
}



/*
struct {
    name : type;
    n1, n2 : t1, t2;
}

// TODO: default values
// TODO: using statements

*/
bool read_struct_type(Token const *& it, unique_ptr<Struct_type>& st)
{
    ASSERT(it != nullptr && *it == STRUCT_KEYWORD);

    st.reset(new Struct_type());

    if (*(++it) != OPEN_BRACE) {
        log_error("Expected \"{\" after \"struct\"",it->context);
        return true;
    }
    ++it; // go past the "{" token
    if (*it == CLOSING_BRACE) {
        it++; // go past the "}" token
        return false; // ok
    }

    do {
        vector<unique_ptr<Typed_identifier>> tid_to_add;

        while (true) {
            if (it->type != Token_type::IDENTIFIER) {
                log_error("Unexpected token in struct: expected identifier",it->context);
                return true;
            }
            unique_ptr<Typed_identifier> tid{new Typed_identifier()};
            tid->identifier_token = it;
            tid_to_add.push_back(move(tid));
            it++; // go past the identifier token
            if (*it == COLON) break;
            if (*it != COMMA) {
                log_error("Unexpected token in struct: expected \",\" between identifiers",it->context);
                return true;
            }
            it++; // go past the "," token
        }
        ASSERT(*it == COLON);
        it++; // go past the ":" token

        bool first = true;
        for (auto& tid : tid_to_add) {
            if (*it == SEMICOLON) {
                log_error("Missing type in struct",it->context);
                return true; // this handles the "too few types" issue
            }
            if (!first) {
                if (*it != COMMA) {
                    log_error("Missing \",\" between types",it->context);
                    return true;
                }
                it++; // go past the "," token
            }
            first = false;

            if (read_type(it,tid->type)) return true;
            st->members.push_back(move(tid));
        }
        if (*it != SEMICOLON) {
            log_error("Unexpected token in struct: expected \";\" after member declaration",it->context);
            return true; // this handles the "too many types" issue
        }
        ++it; // go past the ";" token
    } while (*it != CLOSING_BRACE);

    if (*it != CLOSING_BRACE) {
        log_error("Missing \"}\" at the end of struct",it->context);
        return true;
    }
    it++; // go past the "}" token
    return false; // ok
}



// TODO: if the type already exists somewhere, return a shared ptr to that instead
bool read_type(Token const *& it, shared_ptr<Type_info>& info)
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

    if (*it == FUNCTION_KEYWORD) {
        unique_ptr<Function_type> ft;
        if (read_function_type(it,ft)) return true;
        ASSERT(ft != nullptr);
        info = move(ft);
        return false;
    }

    if (*it == STRUCT_KEYWORD) {
        unique_ptr<Struct_type> st;
        if (read_struct_type(it,st)) return true;
        ASSERT(st != nullptr);
        info = move(st);
        return false;
    }

    log_error("Unexpected token: Expected type",it->context);
    return true;
}








// comma separated list with either pure identifiers, or a parenthesis of "="-separated identifiers
bool read_declaration_lhs_part(Token const*& it, vector<Token const*>& variable_tokens)
{
    ASSERT(it != nullptr);

    if (*it == OPEN_PAREN) {
        while (true) {
            ++it; // go past the "(" or "," token
            if (it->type != Token_type::IDENTIFIER) {
                log_error("Unexpected token in lhs of declaration. Expected identifier.",it->context);
                return true;
            }
            variable_tokens.push_back(it);
            if (*(++it) == CLOSING_PAREN) {
                ++it; // go past the ")" token
                return false; // ok
            }
            if (*it != ASSIGNMENT_TOKEN) {
                log_error("Unexpected token in lhs of declaration. Expected \"=\" between identifiers",it->context);
                return true;
            }
        }
    }

    if (it->type == Token_type::IDENTIFIER) {
        variable_tokens.push_back(it);
        it++; // go past the identifier token
        return false;
    }

    log_error("Unexpected token in lhs of declaration. Expected identifier.",it->context);
    return true;
}

bool read_declaration_lhs(Token const*& it, vector<vector<Token const*>>& variable_tokens)
{
    ASSERT(it != nullptr);
    while(true) {
        vector<Token const*> v;
        if (read_declaration_lhs_part(it,v)) return true;
        ASSERT(!v.empty());
        variable_tokens.push_back(v);
        if (*it != COMMA) return false;
        it++; // go past the "," token
    }
}



bool read_rhs(Token const*& it, vector<unique_ptr<Evaluated_value>>& rhs, Scope* scope, bool force_static = false)
{
    rhs.clear();

    // read rhs
    while (true) {
        unique_ptr<Evaluated_value> val;
        if (read_evaluated_value(it,val,scope,force_static)) return true;
        ASSERT(val != nullptr);
        rhs.push_back(move(val));
        if (*it != COMMA) return false;
        it++; // go past the "," token
    }
}



// returns true if errors
// returns false if syntax seems good, even if not able to deduce types yet. In that case, dependencies are added.
bool read_declaration(Token const*& it, unique_ptr<Dynamic_statement>& statement, Token const * type_token, Token const * assignment_token, Scope* scope, bool force_static)
{
    ASSERT(it != nullptr);
    ASSERT(type_token != nullptr && *type_token == COLON);
    // Read declaration

    unique_ptr<Declaration> declaration{new Declaration()};
    bool errors = false; // we want to continue to rhs in case we find a scope (because we want error messages for that as well) -> don't break on lhs

    // read lhs
    vector<vector<const Token*>> lhs_variable_tokens;
    if (read_declaration_lhs(it,lhs_variable_tokens)) errors = true;

    if (!errors && it != type_token) { // comparing pointers
        log_error("Unexpected token in declaration: expected \":\" directly after lhs. ", it->context);
        add_note("\":\" found here",type_token->context);
        errors = true;
    }
    it = type_token + 1; // after the ":" token

    // read types, if there are any
    vector<shared_ptr<Type_info>> types;
    if (assignment_token == nullptr || it < assignment_token) {
        while (true) {
            shared_ptr<Type_info> type;
            if (read_type(it,type)) {
                errors = true;
                break;
            }
            types.push_back(type);
            if (*it == ASSIGNMENT_TOKEN || *it == SEMICOLON) break;
            if (*it != COMMA) {
                log_error("Unexpected token in declaration: expected \",\" between types",it->context);
                errors = true;
                break;
            }
            ++it; // go past the "," token
        }
    }

    // check if we already know the type
    bool has_type_ids = false;
    if (types.empty()) {
        if (assignment_token == nullptr) {
            log_error("Missing type(s) in declaration",type_token->context);
            errors = true;
        }
    } else if (types.size() != lhs_variable_tokens.size()) {
        ostringstream oss;
        oss << "Type count mismatch in declaration: expected " << lhs_variable_tokens.size() << " types but found only " << types.size();
        log_error(oss.str(),type_token->context);
        errors = true;
    } else {
        has_type_ids = true;
    }

    // add identifiers to scope
    for (int i = 0; i < lhs_variable_tokens.size(); ++i) {
        shared_ptr<Type_info> type{nullptr};
        if (has_type_ids) type = types[i];

        vector<shared_ptr<Typed_identifier>> lhs_part;
        for (Token const* token : lhs_variable_tokens[i]) {

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
                shared_ptr<Typed_identifier> id{new Typed_identifier()};
                id->identifier_token = token;
                id->type = type;
                lhs_part.push_back(id);
                scope->identifiers.push_back(id);
            }
        }
        ASSERT(errors || !lhs_part.empty());
        declaration->lhs.push_back(lhs_part);
    }

    // read rhs if there is one
    if (assignment_token != nullptr) {
        ASSERT(*assignment_token == ASSIGNMENT_TOKEN);
        it = assignment_token + 1; // after the "=" token

        if (*it == SEMICOLON) {
            log_error("Missing values after \"=\"",it->context);
            errors = true;
        } else {
            // read rhs
            if (read_rhs(it,declaration->rhs,scope,force_static)) {
                errors = true;
                add_note("In rhs of declaration assignment",assignment_token->context);
            }
        }
    }

    statement = move(declaration);

    if (errors) return true;

    if (*it != SEMICOLON) {
        log_error("Missing \";\" after declaration",it->context);
        return true;
    }
    it++; // go past the ";" token
    return false; // OK!
}








// comma separated list with either pure identifiers, or a parenthesis of "="-separated identifiers
bool read_assignment_lhs_part(Token const*& it, vector<unique_ptr<Evaluated_variable>>& variables, Scope* scope)
{
    ASSERT(it != nullptr);
    variables.clear();

    if (*it == OPEN_PAREN) {
        while (true) {
            ++it; // go past the "(" or "=" token

            unique_ptr<Evaluated_variable> var;
            if (read_evaluated_variable(it,var,scope,false)) return true;
            ASSERT(var != nullptr)
            variables.push_back(move(var));

            if (*it == CLOSING_PAREN) {
                ++it; // go past the ")" token
                return false; // ok
            }

            if (*it == ASSIGNMENT_TOKEN) {
                log_error("Unexpected token in lhs of declaration. Expected \"=\" between identifiers",it->context);
                return true;
            }
        }
    }

    unique_ptr<Evaluated_variable> var;
    if (read_evaluated_variable(it,var,scope,false)) return true;
    ASSERT(var != nullptr)
    variables.push_back(move(var));
    return false;
}


bool read_assignment_lhs(Token const*& it, vector<vector<unique_ptr<Evaluated_variable>>>& variables, Scope* scope)
{
    ASSERT(it != nullptr);
    while(true) {
        vector<unique_ptr<Evaluated_variable>> v;
        if (read_assignment_lhs_part(it,v,scope)) return true;
        ASSERT(!v.empty());
        variables.push_back(move(v));
        if (*it != COMMA) return false;
    }
}




bool read_assignment(Token const *& it, unique_ptr<Dynamic_statement>& statement, Token const * assignment_token, Scope* scope)
{
    ASSERT(assignment_token != nullptr);
    ASSERT(is_assignment_operator(*assignment_token));
    // Read assignment

    unique_ptr<Assignment> assignment{new Assignment()};
    assignment->assignment_op_token = assignment_token;
    bool errors = false; // we want to continue to rhs in case we find a scope (because we want error messages for that as well) -> don't break on lhs

    // read lhs
    vector<vector<unique_ptr<Evaluated_variable>>> lhs_variables;
    if (read_assignment_lhs(it,lhs_variables,scope)) errors = true;

    if (!errors && it != assignment_token) { // comparing pointers
        log_error("Unexpected token in declaration: expected assignment operator directly after lhs. ", it->context);
        add_note("Assignment operator \""+assignment_token->token+"\" found here",assignment_token->context);
        errors = true;
    }

    // read rhs
    it = assignment_token + 1; // after the assignment token

    if (*it == SEMICOLON) {
        log_error("Missing values after \"=\"",it->context);
        errors = true;
    } else {
        // read rhs
        if (read_rhs(it,assignment->rhs,scope)) {
            errors = true;
            add_note("In rhs of assignment",assignment_token->context);
        }
    }

    if (errors) return true;

    if (*it != SEMICOLON) {
        log_error("Missing \";\" after assignment",it->context);
        return true;
    }
    it++; // go past the ";" token
    statement = move(assignment);
    return false; // OK!
}

bool read_return_statement(Token const*& it, unique_ptr<Dynamic_statement>& statement, Scope* scope)
{
    ASSERT(it != nullptr && *it == RETURN_KEYWORD);
    unique_ptr<Return_statement> rs{new Return_statement()};
    it++; // go past the "return" token
    if (*it != SEMICOLON) {
        if (read_rhs(it,rs->return_values,scope)) return true;
        if (*it != SEMICOLON) {
            log_error("Missing \";\" after return statement",it->context);
            return true;
        }
    }
    it++; // go past the ";" token
    statement = move(rs);
    return false;
}

// if condition { if_true } then { if_false }
bool read_if_clause(Token const*& it, unique_ptr<Dynamic_statement>& statement, Scope* scope)
{
    ASSERT(it != nullptr && *it == IF_KEYWORD);
    unique_ptr<If_clause> is{new If_clause()};
    it++; // go past the if token

    // read condition
    if (read_evaluated_value(it,is->condition,scope,false)) return true;

    // read if true
    if (*it != OPEN_BRACE) {
        // read a single statement?
        log_error("Missing \"{\" after if condition",it->context);
        return true;
    }
    if (read_dynamic_scope(it,is->if_true,scope)) return true;

    // read if false
    if (*it == ELSE_KEYWORD) {
        it++; // go past "else"
        if (*it != OPEN_BRACE) {
            // read a single statement?
            log_error("Missing \"{\" after if condition",it->context);
            return true;
        }
        if (read_dynamic_scope(it,is->if_false,scope)) return true;
    }

    statement = move(is);
    return false;
}

bool read_for_clause(Token const*& it, unique_ptr<Dynamic_statement>& statement, Scope* scope)
{
    ASSERT(it != nullptr && *it == FOR_KEYWORD);
    unique_ptr<For_clause> is{new For_clause()};
    it++; // go past the "for" token
    log_error("read_for_clause nyi",it->context); return true;
    statement = move(is);
    return false;
}

bool read_while_clause(Token const*& it, unique_ptr<Dynamic_statement>& statement, Scope* scope)
{
    ASSERT(it != nullptr && *it == WHILE_KEYWORD);
    unique_ptr<While_clause> wh{new While_clause()};
    it++; // go past the while token

    // read condition
    if (read_evaluated_value(it,wh->condition,scope,false)) return true;

    // read loop body
    if (*it != OPEN_BRACE) {
        // read a single statement?
        log_error("Missing \"{\" after while condition",it->context);
        return true;
    }
    if (read_dynamic_scope(it,wh->loop,scope)) return true;

    statement = move(wh);
    return false;
}


// stops when there are no more ";"-tokens in the current scope
// returns true if error
// should set "it" to after the ";" if possible, even if errors
bool read_statement(Token const*& it, unique_ptr<Dynamic_statement>& statement, Scope* scope, bool force_static)
{
    ASSERT(it != nullptr);

    Token const* type_token = nullptr;
    Token const* assignment_token = nullptr;
    Token const* end_token = nullptr;
    if (examine_statement(it, type_token, assignment_token, end_token)) {
        ASSERT(end_token != nullptr);
        if (*end_token == EOF) it = end_token;
        else it = end_token+1;
        return true; // unable to continue
    }

    if (*it == SEMICOLON) {
        // empty ";" on a line
        log_warning("Additional \";\" ignored",it->context);
        it++; // go past the ";" token
        return read_statement(it,statement,scope,force_static);
    }

    // Possibilities:
    // 1) only ":" token, no assignment
    // 2) both ":" and "=" tokens
    // 3) only assignment token
    // 4) no ":" nor assignment tokens

    bool errors = false;

    if (type_token != nullptr) {
        // declaration
        ASSERT(*type_token == COLON);
        if (read_declaration(it,statement,type_token,assignment_token,scope,force_static)) errors = true;

    } else if (assignment_token != nullptr) {
        // assignment
        ASSERT(is_assignment_operator(*assignment_token));
        // unique_ptr<Assignment> asgn;
        if (read_assignment(it,statement,assignment_token,scope)) errors = true;

    } else if (*it == RETURN_KEYWORD) {
        if (force_static) {
            log_error("Return statements only allowed in dynamic scopes!",it->context);
            errors = true;
        } else {
            if (read_return_statement(it,statement,scope)) errors = true;
        }

    } else if (*it == IF_KEYWORD) {
        if (force_static) {
            log_error("If clause only allowed in dynamic scopes!",it->context);
            errors = true;
        } else {
            if (read_if_clause(it,statement,scope)) errors = true;
        }

    } else if (*it == FOR_KEYWORD) {
        if (force_static) {
            log_error("For loops only allowed in dynamic scopes!",it->context);
            errors = true;
        } else {
            if (read_for_clause(it,statement,scope)) errors = true;
        }

    } else if (*it == WHILE_KEYWORD) {
        if (force_static) {
            log_error("While loops only allowed in dynamic scopes!",it->context);
            errors = true;
        } else {
            if (read_while_clause(it,statement,scope)) errors = true;
        }

    } else {
        // Can for example be a function call or an anonymous scope
        unique_ptr<Evaluated_value> value{nullptr};
        if (read_evaluated_value(it,value,scope,force_static)) errors = true;
        else {
            ASSERT(value != nullptr);
            if (Dynamic_statement* ds = dynamic_cast<Dynamic_statement*>(value.release())) {
                statement.reset(ds);

                if (*it != SEMICOLON) {
                    log_error("Missing \";\" after statement",it->context);
                    errors = true;
                } else {
                    it++; // go past the end token
                }
            } else {
                log_error("Found something that is not a statement where a statement was expected",it->context);
                errors = true;
            }
        }
    }

    if (end_token != nullptr) {
        ASSERT(errors || it == end_token+1);
        it = end_token+1;
    }

    if (!errors) {
        ASSERT(statement != nullptr);
        if (force_static && dynamic_cast<Static_statement*>(statement.get()) == nullptr) {
            log_error("Dynamic statement not allowed in static scope!",it->context);
            errors = true;
        }
    }
    return errors;
}




















/*

// NOTES for handle_imports(...)

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

bool handle_imports(Token const *& it, Scope* scope, Scope* parent_scope)
{
    ASSERT(it != nullptr);
    ASSERT(scope != nullptr);
    ASSERT(parent_scope != nullptr);

    if (*it == OPEN_BRACKET) {
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


bool read_static_scope_statements(Token const*& it, vector<unique_ptr<Static_statement>>& statements, Scope* scope)
{
    ASSERT(it != nullptr);

    bool errors = false;

    while(*it != EOF && !is_closing_symbol(*it)) {
        unique_ptr<Dynamic_statement> statement;
        if (read_statement(it,statement,scope,true)) { // force_static = true
            errors = true;
            if (it == nullptr || *(it-1) != SEMICOLON) return true; // unable to continue
            continue;
        } else {
            ASSERT(statement != nullptr);
            ASSERT(dynamic_cast<Static_statement*>(statement.get()) != nullptr); // this should be forced by the force_static flag
            unique_ptr<Static_statement> ss{dynamic_cast<Static_statement*>(statement.release())};
            statements.push_back(move(ss));
        }
    }
    return errors;
}


bool read_dynamic_scope_statements(Token const*& it, vector<unique_ptr<Dynamic_statement>>& statements, Scope* scope)
{
    ASSERT(it != nullptr);

    bool errors = false;

    while(*it != EOF && !is_closing_symbol(*it)) {
        unique_ptr<Dynamic_statement> statement;
        if (read_statement(it,statement,scope,false)) {
            errors = true;
            if (it == nullptr || *(it-1) != SEMICOLON) return true; // unable to continue
            continue;
        } else {
            statements.push_back(move(statement));
        }
    }
    return errors;
}




bool read_static_scope(Token const *& it, unique_ptr<Static_scope>& scope, Static_scope* parent_scope)
{
    ASSERT(it != nullptr);
    ASSERT(parent_scope != nullptr);

    scope.reset(new Static_scope());

    if (handle_imports(it, scope.get(), parent_scope)) return true;

    ASSERT(*it == OPEN_BRACE);
    it++; // go past the "{" token

    if (read_static_scope_statements(it,scope->statements,scope.get())) return true;
    if (*it != CLOSING_BRACE) {
        log_error("Missing \"}\" at the end of scope",it->context);
        return true;
    }
    it++; // go past the "}" token
    return false;
}

// can be a function body, if statement, etc.
bool read_dynamic_scope(Token const *& it, unique_ptr<Dynamic_scope>& scope, Scope* parent_scope)
{
    ASSERT(it != nullptr);
    ASSERT(parent_scope != nullptr);

    scope.reset(new Dynamic_scope());

    if (handle_imports(it, scope.get(), parent_scope)) {
        return true; // error
    }

    ASSERT(*it == OPEN_BRACE);
    it++; // go past the "{" token


    if (read_dynamic_scope_statements(it,scope->statements,scope.get())) return true;
    if (*it != CLOSING_BRACE) {
        log_error("Missing \"}\" at the end of scope",it->context);
        return true;
    }

    it++; // go past the "}" token
    return false;
}

















Static_scope parse_tokens(const vector<Token>& tokens)
{
    ASSERT(!tokens.empty());
    ASSERT(tokens.back() == EOF);

    Static_scope global_scope{};

    Token const * it = &tokens[0];
    read_static_scope_statements(it,global_scope.statements,&global_scope); // ignore errors. Everything read will be well-formed anyways
    if (*it != EOF) {
        log_error("Extra token after statements in global scope: Found extra tokens after \""+it->token+"\"",it->context);
    }
    return global_scope;
}

Static_scope parse_file(const string& file)
{
    return parse_tokens(get_tokens_from_file(file));
}

Static_scope parse_string(const string& string)
{
    return parse_tokens(get_tokens_from_string(string));
}
