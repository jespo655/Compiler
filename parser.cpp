#include "parser.h"
#include "error_handler.h"
#include "assert.h"
#include <vector>
#include <string>
using namespace std;

unique_ptr<Abs_identifier> read_identifier(const vector<Token>& tokens, Token const * start, Scope* scope, bool allow_function_calls);
unique_ptr<Rhs> read_rhs(const vector<Token>& tokens, Token const * start, Scope* scope, bool allow_function_calls);

unique_ptr<Function_scope> read_function_scope(const vector<Token>& tokens, Token const * start, Scope* scope);
unique_ptr<Scope> read_scope(const vector<Token>& tokens, Token const * start, Scope* parent_scope);



bool is_infix_operator(const Token& t)
{
    if (t.type != Token_type::SYMBOL) return false;

    // TODO: make this function a search in a sorted vector of tokens

    // arithmetic
    if (t.token == "*") return true;
    if (t.token == "/") return true;
    if (t.token == "+") return true;
    if (t.token == "-") return true;
    if (t.token == "%") return true;

    // comparison
    if (t.token == "==") return true;
    if (t.token == "<") return true;
    if (t.token == ">") return true;
    if (t.token == "<=") return true;
    if (t.token == ">=") return true;
    if (t.token == "!=") return true;

    // logic
    if (t.token == "and") return true;
    if (t.token == "or") return true;
    if (t.token == "xor") return true;
    if (t.token == "nor") return true;
    if (t.token == "nand") return true;

    return false;
}


bool is_assignment_operator(const Token& t)
{
    if (t.type != Token_type::SYMBOL) return false;

    // TODO: make this function a search in a sorted vector of tokens

    if (t.token == ":") return true;
    if (t.token == "=") return true;
    if (t.token == "+=") return true;
    if (t.token == "-=") return true;
    if (t.token == "*=") return true;
    if (t.token == "/=") return true;
    if (t.token == "%=") return true;

    return false;
}












// TODO: in the list of tokens, terminate it with an special EOF token

// start must point at the opening thing
// returns pointers to the closing thing
// Token* read_paren(const vector<Token>& tokens, Token* start);
// Token* read_bracket(const vector<Token>& tokens, Token* start);
// Token* read_brace(const vector<Token>& tokens, Token* start);

Token const * read_paren(const std::vector<Token>& tokens, Token const * start);
Token const * read_bracket(const std::vector<Token>& tokens, Token const * start);
Token const * read_brace(const std::vector<Token>& tokens, Token const * start);

Token const * read_token_range_recursive(const vector<Token>& tokens, Token const * start, const string& opening_token, const string& closing_token)
{
    ASSERT(!tokens.empty());

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);
    ASSERT((*start).token == opening_token);

    while(++start <= last) {
        const Token& t = *start;
        if (t.token == closing_token) return start;

        if (t.token == ")" || t.token == "]" || t.token == "}") return start; // error, but it will be handled in wrapper func

        // if (t.token == ")") log_error("Mismatched paren: found unexpected \")\" before \"" + closing_token + "\"",t.context);
        // else if (t.token == "]") log_error("Mismatched bracket: found unexpected \"]\" before \"" + closing_token + "\"",t.context);
        // else if (t.token == "}") log_error("Mismatched brace: found unexpected \"}\" before \"" + closing_token + "\"",t.context);

        if (t.token == "(") start = read_paren(tokens,start);
        else if (t.token == "[") start = read_bracket(tokens,start);
        else if (t.token == "{") start = read_brace(tokens,start);

        if (start == nullptr) return nullptr;
    }
    log_error("Unexpected end of file before \"" + closing_token + "\"",last->context);
    return nullptr;
}

Token const * read_token_range(const vector<Token>& tokens, Token const * start, const string& opening_token, const string& closing_token, const string& range_name)
{
    Token const * end = read_token_range_recursive(tokens,start,opening_token,closing_token);
    if (end == nullptr); // add_note("In "+range_name+" starting at: ",start->context); // uncomment if we want fuller context for the error
    else if (end->token != closing_token) {
        log_error("Mismatched "+range_name+": expected \""+closing_token+"\" before \""+end->token+"\"",end->context);
        add_note("In "+range_name+" starting at: ",start->context);
        return nullptr;
    }
    return end;
}



Token const * read_paren(const vector<Token>& tokens, Token const * start)
{
    return read_token_range(tokens,start,"(",")","paren");
}

Token const * read_bracket(const vector<Token>& tokens, Token const * start)
{
    return read_token_range(tokens,start,"[","]","bracket");
}

Token const * read_brace(const vector<Token>& tokens, Token const * start)
{
    return read_token_range(tokens,start,"{","}","brace");
}











unique_ptr<Capture_group> read_capture_group(const vector<Token>& tokens, Token const * start, Scope* scope)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);

    if (start->token != "[") return nullptr;
    unique_ptr<Capture_group> cg{new Capture_group()};
    cg->start_token = start;
    auto end = read_bracket(tokens,start);
    if (end == nullptr) {
        add_note("Reading capture group");
        return nullptr;
    }
    // todo: read identifiers
    cg->end_token = end;
    return cg;
}









// if condition {} else {}
// if condition {}
// condition = rhs that evaluates to a single boolean
unique_ptr<If_clause> read_if_clause(const vector<Token>& tokens, Token const * start, Scope* scope)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);
    ASSERT(start->token == "if");

    unique_ptr<If_clause> clause{new If_clause()};
    clause->start_token = start;

    clause->condition = read_identifier(tokens,start,scope,true);
    if (clause->condition == nullptr) return nullptr;
    start = clause->condition->end_token + 1;
    if (start > last || start->token != "{") { // capture group not allowed here -> would confuse it with array lookup from condition
        log_error("Expected { after condition in if clause",start->context);
        return nullptr;
    }

    clause->if_true = read_function_scope(tokens, start, scope);
    if (clause->if_true == nullptr) return nullptr;
    clause->end_token = clause->if_true->end_token;

    start = clause->if_true->end_token + 1;
    if (start <= last && start->token == "else")
    {
        ++start;
        if (start > last) {
            log_error("Unexpected end of file in if clause: expected \"{\" after \"else\"",start->context);
            return nullptr;
        } else if (start->token != "{") {
            log_error("Unexpected token in if clause: expected \"{\" after \"else\" but found \""+start->token+"\"",start->context);
            return nullptr;
        }
        clause->if_false = read_function_scope(tokens, start, scope);
        if (clause->if_false == nullptr) return nullptr;
        clause->end_token = clause->if_false->end_token;
    }

    return clause;
}


// [iterator in] range [by step]
// start and end points to after in and before by
unique_ptr<Range> read_range(const vector<Token>& tokens, Token const * start, Scope* scope)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);

    unique_ptr<Range> range{new Range()};
    range->start_token = start;

    ostringstream multiple_in_error{"Multiple \"in\" keywords in range: "};
    ostringstream multiple_by_error{"Multiple \"by\" keywords in range: "};

    while (++start <= last && start->token != "{") {
        if (start->token == "in") {
            if (range->in_token == nullptr) {
                range->in_token = start;
                multiple_in_error << "first declared here: (line " << range->in_token->context.line << ", pos " << range->in_token->context.position << ")";
            } else {
                log_error(multiple_in_error.str(),start->context);
                return nullptr;
            }
            if (range->by_token == nullptr) {
                range->by_token = start;
                multiple_by_error << "first declared here: (line " << range->by_token->context.line << ", pos " << range->by_token->context.position << ")";
            } else {
                log_error(multiple_by_error.str(),start->context);
                return nullptr;
            }
        }
    }

    if (start > last) {
        log_error("Unexpected end of file in range declaration",last->context);
        return nullptr;
    }

    range->end_token = start-1;

    return range;
}



// for it in range by step {}
// for it in range {} // implicit step 1
// for range {} // implicit it _, step 1
// for range by step // implicit it _

// if there is a "in" -> iterator name exists
// if there is a "by" -> step value exists
unique_ptr<For_clause> read_for_clause(const vector<Token>& tokens, Token const * start, Scope* scope)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);
    ASSERT(start->token == "for");

    unique_ptr<For_clause> clause{new For_clause()};
    clause->start_token = start;

    clause->range = read_range(tokens,start,scope);
    if (clause->range == nullptr) return nullptr;
    start = clause->range->end_token + 1;
    if (start > last || start->token != "{") { // capture group not allowed here -> would confuse it with array lookup from condition
        log_error("Expected { after range in for clause",start->context);
        return nullptr;
    }

    clause->loop = read_function_scope(tokens, start, scope);
    if (clause->loop == nullptr) return nullptr;
    clause->end_token = clause->loop->end_token;

    return clause;
}




// while condition {}
unique_ptr<While_clause> read_while_clause(const vector<Token>& tokens, Token const * start, Scope* scope)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);
    ASSERT(start->token == "while");

    unique_ptr<While_clause> clause{new While_clause()};
    clause->start_token = start;

    clause->condition = read_identifier(tokens,start,scope,true);
    if (clause->condition == nullptr) return nullptr;
    start = clause->condition->end_token + 1;
    if (start > last || start->token != "{") { // capture group not allowed here -> would confuse it with array lookup from condition
        log_error("Expected { after condition in while clause",start->context);
        return nullptr;
    }

    clause->loop = read_function_scope(tokens, start, scope);
    if (clause->loop == nullptr) return nullptr;
    clause->end_token = clause->loop->end_token;

    return clause;
}








unique_ptr<Abs_cast> read_cast(const vector<Token>& tokens, Token const * start)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);
    ASSERT(start->token == "_");

    unique_ptr<Abs_cast> cast{new Abs_cast()};
    cast->start_token = start;

    do {
        if ((++start) > last) {
            log_error("Unexpected eof in cast. Expected type identifier after cast token!",last->context);
            return nullptr;
        }
        if (start->type != Token_type::IDENTIFIER) {
            log_error("Expected type identifier after cast token!",start->context);
            return nullptr;
        }
    } while ((++start) <= last && start->token == "_");
    cast->end_token = start-1;
    return cast;
}





/*
    An identifier in lhs can either be:

    A single identifier (just name) (a = ...)
    A typed idenfier (type+name) (int a = ...)
    A "rhs part" (getter, array access etc.) (s.a = ...)
        Exeption: no function calls! They are pure rhs.
        That includes everything that would start with "(". No "(" allowed!

*/

unique_ptr<Abs_identifier> read_lhs_identifier(const vector<Token>& tokens, Token const * start, Scope* scope)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);

    if(start->type != Token_type::IDENTIFIER) {
        log_error("Unexpected token in lhs: expected identifier",start->context);
        return nullptr;
    }

    // if we have 2 identifiers in a row: assume the first one is a type.
    // Allow casts after typed identifiers, but no other operator.
    if (start < last && (start+1)->type == Token_type::IDENTIFIER) {
        unique_ptr<Abs_identifier> id{new Abs_identifier()};
        id->start_token = start;
        id->end_token = start + 1;
        if (start+2 < last && (start+2)->token == "_") {
            id->cast = read_cast(tokens, start+2);
            if (id->cast != nullptr) id->end_token = id->cast->end_token;
        }
        return id;
    }
    // otherwise, use the more elaborate version of read_identifier.
    // However, no function calls are allowed in lhs.
    return read_identifier(tokens, start, scope, false);
}





// TODO: add eof checks
unique_ptr<Lhs_part> read_lhs_part(const vector<Token>& tokens, Token const * start, Scope* scope)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);

    unique_ptr<Lhs_part> part{new Lhs_part()};
    part->start_token = start;

    if(start->type == Token_type::IDENTIFIER) {
        unique_ptr<Abs_identifier> id = read_lhs_identifier(tokens,start,scope);
        if (id == nullptr) {
            add_note("In lhs part started at ",part->start_token->context);
            return nullptr;
        }
        part->end_token = id->end_token + 1;
        part->identifiers.push_back(move(id));
        return part;

    } else if (start->token == "(") {
        part->end_token = read_paren(tokens, start); // so we can return without worrys
        if (part->end_token == nullptr) {
            add_note("In lhs part started at ",part->start_token->context);
            return nullptr;
        }

        if ((++start) > last) {
            log_error("Unexpected end of file in lhs part",last->context);
            return nullptr;
        }
        if (start->token == ")") {
            log_error("Found empty lhs part. Identifiers in each position is required. ",part->start_token->context);
            return nullptr; // empty part
        }

        do {
            if (start->type != Token_type::IDENTIFIER) {
                log_error("Expected identifier in LHS but found unknown token "+start->token,start->context);
                return part;
            }
            unique_ptr<Abs_identifier> id = read_lhs_identifier(tokens,start,scope);
            if (id == nullptr) {
                add_note("In lhs part started at ",part->start_token->context);
                return nullptr;
            }
            start = id->end_token + 1;
            part->identifiers.push_back(move(id));
            if (start > last) return part;

            if (start->token == ")") return part; // ok
            if (start->token != "=") {
                log_error("Unexpected token after identifier in lhs part Expected \")\" or \"=\" but found "+start->token,start->context);
                return part;
            }
        } while ((++start) > last);
        log_error("Unexpected end of file in lhs part.",last->context);
        return nullptr;
    }
    log_error("Unexpected token in the beginning of lhs part: "+start->token,start->context);
    return nullptr;
}



unique_ptr<Lhs> read_lhs(const vector<Token>& tokens, Token const * start, Scope* scope)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);

    unique_ptr<Lhs> lhs{new Lhs()};
    lhs->start_token = start;

    if (start->type != Token_type::IDENTIFIER && start->token != "(" ) {
        log_error("Unexpected token in the beginning of lhs",start->context);
        return nullptr;
    }

    do {
        unique_ptr<Lhs_part> part = read_lhs_part(tokens,start,scope);
        if (part == nullptr) return nullptr;
        lhs->end_token = part->end_token;
        start = part->end_token + 1;
        lhs->parts.push_back(move(part));
        if (start > last || start->token != ",") return lhs;
    } while ((++start) <= last);
    log_error("Unexpected end of file in lhs",last->context);
    return nullptr;
}






// statement:
// Assignment
// Function_call
// If_clause
// For_clause
// While_clause

unique_ptr<Declaration> read_declaration(const vector<Token>& tokens, Token const * start, Scope* scope)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);

    unique_ptr<Declaration> decl{new Declaration()};
    decl->start_token = start;

    while (start < last) {

        if (start->type != Token_type::IDENTIFIER || (start+1)->type != Token_type::IDENTIFIER) {
            log_error("Unexpected token in declaration: expected type name and variable name",start->context);
            return nullptr;
        }

        unique_ptr<Abs_identifier> id{new Abs_identifier()};
        id->start_token = start;
        id->end_token = start+1;
        decl->identifiers.push_back(move(id));

        if ((start+2) > last || (start+2)->token != ",") {
            // ok
            decl->end_token = start+1;
            return decl;
        }
        start+=3;
    }
    log_error("Unexpected end of file in declaration",last->context);
    return nullptr;
}


unique_ptr<Statement> read_statement(const vector<Token>& tokens, Token const * start, Scope* scope, bool allow_function_calls)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);

    // check if, for, while
    if (allow_function_calls) {
        if (start->token == "if") return read_if_clause(tokens,start,scope);
        if (start->token == "for") return read_for_clause(tokens,start,scope);
        if (start->token == "while") return read_while_clause(tokens,start,scope);
    } else if (start->token == "if" || start->token == "for" || start->token == "while") {
        log_error("If, for and while clauses are not allowed in context where function calls are not allowed",start->context);
        return nullptr;
    }
    // check for end of statement
    // read tokens until ";"
    // if we find a out of parens assignment operator then it's an assignment -> read lhs then rhs
    // if not, then just read rhs

    Token const* assignment_op_token = nullptr;
    Token const* it = start;

    while((++it) <= last) {
        if (it->token == ";") break;
        else if (it->token == "(") it = read_paren(tokens,it);
        else if (it->token == "[") it = read_bracket(tokens,it);
        else if (it->token == "{") it = read_brace(tokens,it);
        else if (is_assignment_operator(*it)) {
            if (assignment_op_token != nullptr) {
                log_error("Multiple assignment operators in statement",it->context);
                add_note("First found here: ",assignment_op_token->context);
                add_note("Expected \";\" after each assignment.");
                return nullptr;
            }
            assignment_op_token = it;
        }
        if (it == nullptr) return nullptr; // from read_paren and Co.
    }
    if (it > last) {
        log_error("Unexpected end of file: expected \";\" after statement",last->context);
        add_note("Statement started here: ",start->context);
    }

    if (assignment_op_token != nullptr) {
        unique_ptr<Assignment> asgn{new Assignment};
        asgn->start_token = start;
        asgn->end_token = it; // ";" token
        asgn->lhs = read_lhs(tokens,start,scope);
        if (asgn->lhs == nullptr) return nullptr;
        asgn->rhs = read_rhs(tokens,assignment_op_token+1,scope,allow_function_calls);
        if (asgn->rhs == nullptr) return nullptr;
        ASSERT(asgn->lhs->end_token == assignment_op_token-1);
        ASSERT(asgn->rhs->end_token == it-1);
        return asgn;
    }

    // check for declaration
    if (start < last && start->type == Token_type::IDENTIFIER && (start+1)->type == Token_type::IDENTIFIER) {
        auto decl = read_declaration(tokens,start,scope);
        if (decl == nullptr) return nullptr;
        ASSERT(decl->end_token == it-1);
        return decl;
    }

    // check for function call
    if (allow_function_calls) {
        auto id = read_identifier(tokens,start,scope,true);
        if (id == nullptr) return nullptr;

        ASSERT(id->end_token == it-1); // it should end right before the ";" token

        if (Function_call* fc = dynamic_cast<Function_call*>(id.get())) {
            unique_ptr<Function_call> fcp{dynamic_cast<Function_call*>(id.release())};
            return fcp;
        }
    }

    log_error("Found something that is not a statement in a context where a statement is expected",start->context);
    return nullptr;

    // things that are not assignments:
    // * declarations
    //      int a;
    //      float b, float c;
    // always a comma separated list of type+identifier. Casts are not allowed

    // []{}; anonymous scope
    // []{}.foo()_bar().baz     // function call / cast / getter chain (can start with either an identifier or an anonymous scope)
}




// read start identifier
// can be an identifier token, an anonymous scope, or a parenthesis that evaluates to one of those things
// Can not include type. Be wary of special cases in lhs.
unique_ptr<Abs_identifier> read_identifier(const vector<Token>& tokens, Token const * start, Scope* scope, bool allow_function_calls)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);

    unique_ptr<Abs_identifier> id{new Abs_identifier};
    id->start_token = start;

    if (start->type == Token_type::SYMBOL && start->token == "(") {
        if (!allow_function_calls) {
            log_error("Unexpected parens in a context where function calls are not allowed",start->context);
            return nullptr;
        }

        id = read_rhs(tokens, start+1, scope, allow_function_calls);
        if (id == nullptr) return nullptr;
        if (id->end_token >= last || (id->end_token+1)->token != ")") {
            log_error("Missing \")\" at the end of parens-enclosed rhs part",(id->end_token+1)->context);
            add_note("Parens started here ",start->context);
            return nullptr;
        }
        start = id->end_token + 2; // after the ")" token
    } else if (start->type == Token_type::SYMBOL && start->token == "[" || start->token == "{") {
        id = read_scope(tokens, start, scope);
    } else if (start->type == Token_type::IDENTIFIER) {
        id->end_token = start;
    } else {
        log_error("Unexpected token in rhs: expected identifier but found unknown token "+start->token,start->context);
        return nullptr;
    }
    start++;

    while(start <= last) {

        if (start->token == "(") {
            if (!allow_function_calls) {
                log_error("Unexpected parens in a context where function calls are not allowed",start->context);
                return nullptr;
            }
            // read function call
            unique_ptr<Function_call> fc{new Function_call()};
            fc->start_token = id->start_token;
            fc->function_identifier = read_rhs(tokens,start,scope,allow_function_calls);
            if (fc->function_identifier == nullptr) return nullptr;
            fc->end_token = fc->function_identifier->end_token + 1; // ")" token
            if (fc->end_token->token != ")") {
                log_error("Unexpected token after function call parameters. Expected \")\".", fc->end_token->context);
                return nullptr;
            }
            start = fc->function_identifier->end_token + 2; // after the ")" token
            fc->function_identifier = move(id);
            id = move(fc);
        } else if (start->token == ".") {
            // read getter
            if ((++start) > last) break; // eof
            if (start->type != Token_type::IDENTIFIER) {
                log_error("Unexpected token after getter. Expected identifier but found unknown token "+start->token,start->context);
                return nullptr;
            }
            unique_ptr<Getter> g{new Getter()};
            g->start_token = id->start_token;
            g->end_token = start;
            g->struct_identifier = move(id);
            g->data_identifier_token = start;
            id = move(g);
            ++start;
        } else if (start->token == "_") {
            // read cast chain
            id->cast = read_cast(tokens,start);
            if (id->cast == nullptr) return nullptr;
            id->end_token = id->cast->end_token;
            start = id->cast->end_token + 1 ;

        } else if (is_infix_operator(*start)) {

            // for now: do the naive non-priority stuff
            // later: add priority
            unique_ptr<Infix_op> inf{new Infix_op()};
            inf->start_token = id->start_token;
            inf->lhs = move(id);
            inf->op_token = start;
            if ((++start) > last) break; // eof
            inf->rhs = read_identifier(tokens,start,scope,allow_function_calls);
            if (inf->rhs == nullptr) return nullptr;
            inf->end_token = inf->rhs->end_token;
            id = move(id);

        } else {
            // unknown token. Return;
            return id;
        }
    }
    log_error("Unexpected end of file.",last->context);
    return nullptr;
}



unique_ptr<Rhs> read_rhs(const vector<Token>& tokens, Token const * start, Scope* scope, bool allow_function_calls)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);

    unique_ptr<Rhs> rhs{new Rhs()};

    do {
        unique_ptr<Abs_identifier> id = read_identifier(tokens,start,scope,allow_function_calls);
        if (id == nullptr) return rhs;
        start = id->end_token + 1;
        rhs->identifiers.push_back(move(id));

        if (start > last) break; // eof
        if (start->token != ",") {
            // end of rhs - ok
            rhs->end_token = start-1;
            return rhs;
        }

    } while ((++start) > last);

    log_error("Unexpected end of file in rhs.",last->context);
    return nullptr;

}



/*


a = foo().bar().baz;



function call:

    foo
    ()

getter, "."
    foo()
    .
    bar

function call
    foo().bar
    ()

getter, "."
    foo().bar()
    .
    baz




"("
unique_ptr<Function_call> fc{new Function_call};
fc->function_identifier = "foo"-token;
fc->arguments = void

"."
unique_ptr<Getter> g{new Getter};
g->struct_identifier = move(fc);
g->data_identifier = "bar"-token;

"("
unique_ptr<Function_call> fc2{new Function_call};
fc2->function_identifier = move(g);
fc2->arguments = void

"."
unique_ptr<Getter> g2{new Getter};
g2->struct_identifier = move(fc2);
g2->data_identifier = "baz"-token;

";"
return g2;

*/










/*
Assignment:

LHS = RHS

a = fn(){} // a blir funktionen fn(){} (type "fn()")

a = fn(){}(); // a blir det returnerade värdet från funktionen fn(){} (dvs void -> error: assignment mismatch: LHS has 1 identifier but RHS has 0 values)

S = struct{ int i; } // S blir typen "type" ("struct" eller "struct_type" för att kompilatorn ska veta att det är en struct?)
S b;

c = b.i; // c blir typ int (dependant on type of member i of struct s)

d = (S e).i; // parentesen evalueras först -> e blir en S-struct -> d blir en int
    (d är depednant on type of i som är dependat on type of e som är dependant on user defined type S)

d = (e = S()).i; // samma som ovan, men e är dependant on function S (som definieras tillsammans med struct type S)


*/




unique_ptr<Function_scope> read_function_scope(const vector<Token>& tokens, Token const * start, Scope* scope)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);
    ASSERT(start->token == "[" || start->token == "{");

    unique_ptr<Function_scope> fs{new Function_scope()};

    fs->capture_group = read_capture_group(tokens,start,scope);
    if (fs->capture_group != nullptr) {
        start = fs->capture_group->end_token + 1;
        if (start->token != "{") {
            log_error("Expected \"{\" after capture group",start->context);
            return nullptr;
        }
    } else {
        fs->parent_scope = scope; // only import parent scope if there is no capture group
    }

    fs->start_token = start;
    auto end = read_brace(tokens,start);
    if (end == nullptr) {
        add_note("Reading scope");
        return nullptr;
    }
    // todo: read statements
    fs->end_token = end;
    return fs;
}




unique_ptr<Scope> read_scope(const vector<Token>& tokens, Token const * start, Scope* parent_scope)
{
    ASSERT(false, "read_scope not yet implemented");
    return nullptr;
    // unique_ptr<Function_scope> fs = read_function_scope(tokens, start, parent_scope);
    // if (fs == nullptr) return nullptr;
    // return new Scope(move(fs));




/*

a scope is filled with identifiers that gets declared
each identifier should have a declaration context that is a statement


read statements:
    assert that they are declarations or assignments (not function calls)
    add the statement to the scope. (unique_ptr)
    add all declared identifiers to the scope. Each identifier has a (weak) pointer to the statement.









todo: read_rhs med bool allow_function_call





*/































}

