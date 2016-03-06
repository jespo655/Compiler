#ifndef parser_h
#define parser_h

#include <vector>
#include "lexer.h"
#include <memory> // std::unique_ptr



/*

Pass 1:
    Go through all tokens and check for matching (), [], {}.
    Create an outline of what has to be parsed:
        A list of scopes, each with a list of identifiers. Check that each identifier only is declared once. (TODO: scopes can continue in different file?)
        A list of high level function clauses.


Pass 2:
    For each scope: read statements.
    Assign each identifier a type and a value. (no need to check that it is only assigned once, that is already covered in pass 1).
    Multiple passes might be needed to resolve all identifiers.
        Keep a list of unresolved identifiers and keep going until empty.
        If the list didn't reduce in size during a pass -> compile error, exit.

    Functions are not allowed in static scopes. -> we should always be able to deduce the value of each identifier. (unless #run? -> todo later)


Pass 3:
    For each high level function: read statements
        Local_scope: changes as we traverse through statements (identifiers are added)
        Each identifier used has to be declared in a reachable scope (no definition necessary yet)

    At this point we could already build byte code:
        just start with main(), and build functions as we use them
        -> the local scope identifiers always has the correct values

        TODO: inline or not

    Keep a list of which functions are left to build.
    If we dont reach all functions through main -> build the rest anyways to check for compile errors (but just check types, dont assign any values)
        This step can be an compile option?



Note:
    We could only resolve unknown identifiers as they are used in pass3.
    That would possibly increase compile time, by skipping lots of stuff.

    However, that would allow for undefined variables in the code, or functions that would contain compile errors.
    That is NOT something we want.


TODO:
    tokenizer
    vector<Token> tokenize_file(file f);
    vector<Token> tokenize_string(string s);

    in pass 2: we can add stuff to the compilation queue. (#build?)
        every build target is build in the global scope

*/










// For paren text. TODO: remove
Token const * read_paren(const std::vector<Token>& tokens, Token const * start);
Token const * read_bracket(const std::vector<Token>& tokens, Token const * start);
Token const * read_brace(const std::vector<Token>& tokens, Token const * start);



struct Abs_syntax
{
    const Token* start_token = nullptr;
    const Token* end_token = nullptr;
    Abs_syntax() {}
    virtual ~Abs_syntax() {}
};


struct Abs_cast : Abs_syntax
{

};

struct Abs_identifier : virtual Abs_syntax
{
    std::unique_ptr<Abs_cast> cast{nullptr};
};



struct Statement : virtual Abs_syntax
{

};


struct Capture_group : Abs_syntax
{
    std::vector<Abs_identifier> identifiers; // For now: capture by value only. TODO: capture by reference ("points" to identifiers in other scopes)
};




struct Scope : Abs_identifier
{
    std::vector<Scope*> imported_scopes;
    std::unique_ptr<Capture_group> capture_group = nullptr;
    std::vector<Abs_identifier> identifiers;

    // Scope() = default;
    // Scope(const Scope&) = default;
    // Scope(Scope&&) = default;
    // Scope(Function_scope&& fs) : Abs_identifier{fs}, capture_group{std::move(fs.capture_group)} { imported_scopes.push_back(fs.parent_scope); }
};


struct Function_scope : Abs_syntax
{
    Scope* parent_scope = nullptr;
    std::unique_ptr<Capture_group> capture_group = nullptr;
    std::vector<std::unique_ptr<Statement>> statements;
};


struct Local_scope : Scope
{
    std::vector<std::unique_ptr<Statement>> defer_statements;
};



/* // TODO
// a named function is an Identifier with the name and type of the function, with the value_ptr begin a pointer to a Function struct
struct Function : Abs_syntax
{
    // NOTE: the order in all these vectors is very important
    std::vector<Abs_identifier> in_parameters;
    std::vector<Abs_identifier> out_parameters;
    std::unique_ptr<Function_scope> body;
    // in the next step a local scope is created from in and out parameters
    //  the local scope changes for each statement
};
*/


struct Lhs_part : Abs_syntax
{
    std::vector<std::unique_ptr<Abs_identifier>> identifiers; // must have the same type (after cast)
    // Type_info* get_type(); // nullptr if unknown
};

struct Lhs : Abs_syntax
{
    std::vector<std::unique_ptr<Lhs_part>> parts;
};



struct Rhs : Abs_identifier
{
    // a comma separated list of rhs parts
    // each part is something that can be evaluated to one or more values
    std::vector<std::unique_ptr<Abs_identifier>> identifiers;
};

struct Declaration : Statement
{
    std::unique_ptr<Lhs> lhs;
};

struct Assignment : Statement
{
    std::unique_ptr<Lhs> lhs;
    std::unique_ptr<Rhs> rhs;
};

struct Function_call : Abs_identifier, Statement
{
    std::unique_ptr<Abs_identifier> function_identifier;
    std::unique_ptr<Rhs> arguments;
};

struct Getter : Abs_identifier
{
    std::unique_ptr<Abs_identifier> struct_identifier;
    Token const* data_identifier_token;
};

struct Infix_op : Abs_identifier
{
    std::unique_ptr<Abs_identifier> lhs;
    std::unique_ptr<Abs_identifier> rhs;
    Token const * op_token;
};

struct If_clause : Statement
{
    std::unique_ptr<Abs_identifier> condition; // must evaluate to exactly one bool
    std::unique_ptr<Function_scope> if_true;
    std::unique_ptr<Function_scope> if_false;
};


struct Range : Abs_syntax
{
    // can either be a list identifier or a range declaration a..b
    Abs_identifier iterator; // value = step
    const Token* in_token = nullptr;
    const Token* by_token = nullptr;
};


struct For_clause : Statement
{
    std::unique_ptr<Range> range;
    std::unique_ptr<Function_scope> loop;
};

struct While_clause : Statement
{
    std::unique_ptr<Abs_identifier> condition; // must evaluate to exactly one bool
    std::unique_ptr<Function_scope> loop;
};



#endif
