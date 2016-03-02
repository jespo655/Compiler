#include "parser.h"
#include "error_handler.h"
#include "assert.h"
#include <vector>
#include <string>
using namespace std;

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











unique_ptr<Capture_group> read_capture_group(const vector<Token>& tokens, Token const * start)
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





unique_ptr<Function_scope> read_function_scope(const vector<Token>& tokens, Token const * start, Scope* parent_scope = nullptr)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);
    ASSERT(start->token == "[" || start->token == "{");

    unique_ptr<Function_scope> fs{new Function_scope()};

    fs->capture_group = read_capture_group(tokens,start);
    if (fs->capture_group != nullptr) {
        start = fs->capture_group->end_token + 1;
        if (start->token != "{") {
            log_error("Expected \"{\" after capture group",start->context);
            return nullptr;
        }
    } else {
        fs->parent_scope = parent_scope; // only import parent scope if there is no capture group
    }

    fs->start_token = start;
    auto end = read_brace(tokens,start);
    if (end == nullptr) {
        add_note("Reading function scope");
        return nullptr;
    }
    // todo: read statements
    fs->end_token = end;
    return fs;
}







// if condition {} else {}
// if condition {}
// condition = rhs that evaluates to a single boolean
unique_ptr<If_clause> read_if_clause(const vector<Token>& tokens, Token const * start)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);
    ASSERT(start->token == "if");

    // find next {}. the clause context is from the end of if to just before the {
    unique_ptr<If_clause> clause{new If_clause()};
    clause->start_token = start;
    clause->condition.start_token = (start+1);
    while (start <= last && start->token != "{") ++start;
    clause->condition.end_token = (start-1);

    if (start > last) {
        log_error("Unexpected end of file in if clause: expected \"{\" after condition",start->context);
        add_note("Start of if clause: ",clause->start_token->context);
        return nullptr;
    }

    clause->if_true = read_function_scope(tokens, start);
    if (clause->if_true == nullptr) {
        add_note("In if clause started at ",clause->start_token->context);
        return nullptr;
    }
    clause->end_token = clause->if_true->end_token;

    // todo: read statements to if_true

    start = clause->if_true->end_token + 1;
    if (start <= last && start->token == "else")
    {
        ++start;
        if (start > last) {
            log_error("Unexpected end of file in if clause: expected \"{\" after \"else\"",start->context);
            // add_note("Start of if clause: ",(clause->condition.start_token-1)->context);
            return nullptr;
        } else if (start->token != "{") {
            log_error("Unexpected token in if clause: expected \"{\" after \"else\" but found \""+start->token+"\"",start->context);
            // add_note("Start of if clause: ",(clause->condition.start_token-1)->context);
            return nullptr;
        }
        clause->if_false = read_function_scope(tokens, start);
        if (clause->if_false == nullptr) {
            add_note("In else clause started at ",(start-1)->context);
            return nullptr;
        }
        clause->end_token = clause->if_false->end_token;

        // todo: read statements to if_false
    }

    return clause;
}


// [iterator in] range [by step]
// start and end points to after in and before by
unique_ptr<Range> read_range(const vector<Token>& tokens, Token const * start, const string& expected_terminator)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);

    unique_ptr<Range> range{new Range()};
    range->start_token = start;

    ostringstream multiple_in_error{"Multiple \"in\" keywords in range: "};
    ostringstream multiple_by_error{"Multiple \"by\" keywords in range: "};

    while (++start <= last && start->token != expected_terminator) {
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
        log_error("Unexpected end of file: expected "+expected_terminator+" at the end of range declaration",last->context);
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
unique_ptr<For_clause> read_for_clause(const vector<Token>& tokens, Token const * start)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);
    ASSERT(start->token == "for");

    // find next {}. the clause context is from the end of if to just before the {
    unique_ptr<For_clause> clause{new For_clause()};
    clause->start_token = start;

    clause->range = read_range(tokens,start,"{");
    if (clause->range == nullptr) {
        add_note("In for clause started at: ",clause->start_token->context);
        return nullptr;
    }

    start = clause->range->end_token + 1;

    if (start > last) {
        log_error("Unexpected end of file in for clause: expected \"{\" after range declaration",start->context);
        add_note("Start of for clause: ",clause->start_token->context);
        return nullptr;
    }

    clause->loop->start_token = start;
    start = read_brace(tokens,start);
    if (start == nullptr) {
        add_note("In for clause started at ",clause->start_token->context);
        return nullptr;
    }
    clause->loop->end_token = start;
    // todo: read statements to loop

    clause->end_token = start;
    return clause;

}




// while condition {}
unique_ptr<While_clause> read_while_clause(const vector<Token>& tokens, Token const * start)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);
    ASSERT(start->token == "if");

    // find next {}. the clause context is from the end of if to just before the {
    unique_ptr<While_clause> clause{new While_clause()};
    clause->start_token = start;
    clause->condition.start_token = (start+1);
    while (start <= last && start->token != "{") ++start;
    clause->condition.end_token = (start-1);

    if (start > last) {
        log_error("Unexpected end of file in while clause: expected \"{\" after condition",start->context);
        add_note("Start of if clause: ",clause->start_token->context);
        return nullptr;
    }

    clause->loop->start_token = start;
    start = read_brace(tokens,start);
    if (start == nullptr) {
        add_note("In if clause started at ",clause->start_token->context);
        return nullptr;
    }
    clause->loop->end_token = start;

    // todo: read statements to loop

    clause->end_token = start;
    return clause;
}








// statement:
// Assignment
// Function_call
// If_clause
// For_clause
// While_clause

unique_ptr<Statement> read_statement(const vector<Token>& tokens, Token const * start)
{
    ASSERT(start != nullptr);

    const Token* last = &tokens.back();

    ASSERT(start >= &*tokens.begin());
    ASSERT(start <= last);

    // check if, for, while
    if (start->token == "if") return read_if_clause(tokens,start);
    if (start->token == "for") return read_for_clause(tokens,start);
    if (start->token == "while") return read_while_clause(tokens,start);

    // Read lhs part
    // next token is "," -> continue reading LHS
    // next token is assignment operator -> read RHS
    // next token is ( -> read function (requires that the only token in LHS is a single identifier)
}



