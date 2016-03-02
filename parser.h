#ifndef parser_h
#define parser_h

#include <vector>
#include "lexer.h"
#include <memory> // unique_ptr



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











// Token const * read_paren(const std::vector<Token>& tokens, Token const * start);
// Token const * read_bracket(const std::vector<Token>& tokens, Token const * start);
// Token const * read_brace(const std::vector<Token>& tokens, Token const * start);



struct Abs_syntax
{
    const Token* start_token = nullptr;
    const Token* end_token = nullptr;
    Abs_syntax() {}
    virtual ~Abs_syntax() {}
};




struct Type_info
{
    std::string type;
    unsigned int size = 0; // bytes

    Type_info(const std::string& t = "", unsigned int s = 0) : type{t}, size{s} {}
    bool operator<(const Type_info& o) const { return type < o.type; } // for sorting
    bool operator==(const Type_info& o) const { return type == o.type && size == o.size; }
    bool operator!=(const Type_info& o) const { return !(*this == o); }
    virtual ~Type_info() {}
    virtual Type_info& get_casted_type() { return *this; }
};


struct Castable_type : Type_info
{
    Type_info* casted_type = nullptr; // owned by the castable-type

    Castable_type() {}
    Castable_type(const Type_info& t) : Type_info{t} {}
    Castable_type(const Castable_type& o) : Type_info{o} { deep_copy_cast_type_ptr(o); }
    Castable_type(Castable_type&& o) : Type_info{std::move(o)}, casted_type{o.casted_type} { o.casted_type = nullptr; }
    Castable_type& operator=(const Castable_type& o) { type = o.type; size = o.size; delete casted_type; deep_copy_cast_type_ptr(o); }
    Castable_type& operator=(Castable_type&& o) { type = std::move(o.type); size = o.size; casted_type = o.casted_type; o.casted_type = nullptr; }
    Type_info& get_casted_type() { return (casted_type==nullptr)? *this : casted_type->get_casted_type(); }

private:
    void deep_copy_cast_type_ptr(const Castable_type& o);
};





struct Identifier
{
    std::string name;
    Type_info type;
    void* value_ptr; // mallocs and frees with construction/destruction of the object. Don't change this!
    Token_context context; // definiton context

    Identifier(); // no name and undefined type
    Identifier(const std::string& s, const Type_info& t); // malloc value_ptr
    virtual ~Identifier(); // free value_ptr
    bool operator<(const Identifier& o) const { return name < o.name; } // for sorting
    bool operator==(const Identifier& o) const { return name == o.name; }
    Identifier(const Identifier&);
    Identifier(Identifier&&);
    Identifier& operator=(const Identifier&);
    Identifier& operator=(Identifier&&);
};


struct Statement : Abs_syntax
{
    // TODO: alla som har listor av statements måste allockera och fria dem själv
};


struct Capture_group : Abs_syntax
{
    std::vector<Identifier*> identifiers; // points to identifiers in other scopes
};

struct Scope : Abs_syntax
{
    std::unique_ptr<Capture_group> capture_group = nullptr;
    std::vector<Identifier> identifiers;
    std::vector<Scope*> imported_scopes;
};


struct Local_scope : Scope
{
    std::vector<std::unique_ptr<Statement>> defer_statements;
};


struct Function_scope : Abs_syntax
{
    Scope* parent_scope = nullptr;
    std::unique_ptr<Capture_group> capture_group = nullptr;
    std::vector<std::unique_ptr<Statement>> statements;
};


// a named function is an Identifier with the name and type of the function, with the value_ptr begin a pointer to a Function struct
struct Function : Abs_syntax
{
    // NOTE: the order in all these vectors is very important
    std::vector<Identifier> in_parameters;
    std::vector<Identifier> out_parameters;
    std::unique_ptr<Function_scope> body;
    // in the next step a local scope is created from in and out parameters
    //  the local scope changes for each statement
};


struct Lhs_part : Abs_syntax
{
    std::vector<Identifier> identifier; // must have the same type (after cast)
    Type_info* get_type(); // nullptr if unknown
};

struct Lhs : Abs_syntax
{
    std::vector<Lhs_part> identifiers;
};


struct Rhs : Abs_syntax
{
    // something that can be evaluated to one or more values
    // std::vector<Identifier> identifiers; // ???
};


struct Assignment : Statement
{
    Lhs lhs;
    Rhs rhs;
};

struct Function_call : Statement
{
    std::string fn_name;
    Rhs arguments;
};

struct If_clause : Statement
{
    Rhs condition; // must evaluate to exactly one bool
    std::unique_ptr<Function_scope> if_true;
    std::unique_ptr<Function_scope> if_false;
};


struct Range : Abs_syntax
{
    // can either be a list identifier or a range declaration a..b
    Identifier iterator; // value = step
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
    Rhs condition; // must evaluate to exactly one bool
    std::unique_ptr<Function_scope> loop;
};



#endif
