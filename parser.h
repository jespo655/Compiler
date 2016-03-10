#ifndef parser_h
#define parser_h

#include <vector>
#include "lexer.h"
#include <memory> // std::unique_ptr



std::unique_ptr<Scope> parse_file(const std::string& file);
std::unique_ptr<Scope> parse_string(const std::string& string);



struct Token_range
{
    Token const* start_token;
    Token const* end_token;
    virtual ~Token_range();
};





// Evaluated value: Anything that can hold a value.
struct Evaluated_value
{
    virtual ~Evaluated_value();
};

// Evaluated variable: Anything that can be assigned a value. I.E. not literals.
// Anything that can be assigned a value can also hold a value.
struct Evaluated_variable : Evaluated_value
{
    virtual ~Evaluated_value();
};

struct Literal : Evaluated_value
{
    Token const * literal_token;
    virtual ~Literal();
};

struct Identifier : Evaluated_variable
{
    Token const * identifier_token;
    virtual ~Identifier();
};

struct Infix_op : Evaluated_value
{
    unique_ptr<Evaluated_variable> lhs;
    Token const * op_token;
    unique_ptr<Evaluated_variable> rhs;
};

// function call: starts with "("
struct Function_call : Evaluated_variable
{
    unique_ptr<Evaluated_variable> function_identifier;
    std::vector<std::unique_ptr<Evaluated_value>> arguments;
};

// getter: starts with "."
struct Getter : Evaluated_variable
{
    unique_ptr<Evaluated_variable> struct_identifier;
    Token const * data_identifier_token;
};

// cast: starts with "_"
struct Cast : Evaluated_variable
{
    unique_ptr<Evaluated_value> casted_thing; // todo: rename
    Type_info const* casted_type;
};

// array lookup: starts with "["
struct Array_lookup : Evaluated_variable
{
    unique_ptr<Evaluated_variable> array_identifier;
    unique_ptr<Evaluated_value> position;
};








// Dynamic_statement: any statement that can be evaluated in a dynamic scope (function)
struct Dynamic_statement
{
    virtual ~Dynamic_statement();
};

// A Static_statement is a statement that can be evaluated in a static scope.
// Any static statement can also be performed in a dynamic context.
struct Static_statement : Dynamic_statement
{
    virtual ~Static_statement();
};

// Definition: any statement including ":" and maybe "="
struct Definition : Static_statement
{
    std::vector<std::vector<Token const*>> variable_name_tokens{};
    std::vector<Type_info const*> types{}; // has to be either empty or exactly the same length as variable_name_tokens
    std::vector<std::unique_ptr<Evaluated_value>> rhs{}; // can be empty if types is not. One part can be a function that returns several values. Check that the count matches when all top-level functions are resolved.
};


// Assignment: any assignment not including ":"
struct Assignment : Dynamic_statement
{
    std::vector<std::unique_ptr<Evaluated_variable>> lhs{};
    Token const * assignment_op_token{nullptr};
    std::vector<std::unique_ptr<Evaluated_value>> rhs{};
};

struct If_clause : Dynamic_statement
{
    std::unique_ptr<Evaluated_value> condition{nullptr}; // has to evaluate to exactly one bool
    std::vector<std::unique_ptr<Dynamic_statement>> if_true{};
    std::vector<std::unique_ptr<Dynamic_statement>> if_false{};
};

struct While_clause : Dynamic_statement
{
    std::unique_ptr<Evaluated_value> condition{nullptr};
    std::vector<std::unique_ptr<Dynamic_statement>> loop{};
};


struct Range
{
    virtual ~Range();
};

// Array?
struct List : Range, Identifier
{
    // TODO: add stuff @arrays
};

struct Literal_range : Range, Evaluated_value
{
    unique_ptr<Literal> start{};
    unique_ptr<Literal> end{};
};

struct Evaluated_range : Range
{
    unique_ptr<Evaluated_variable> start{nullptr};
    unique_ptr<Evaluated_variable> end{nullptr};
};

struct For_clause : Dynamic_statement
{
    Token const * iterator_name_token{nullptr}; // might not exist
    std::unique_ptr<Range> range{nullptr}; // has to exist
    std::unique_ptr<Evaluated_value> step{nullptr}; // might not exist
    std::vector<std::unique_ptr<Dynamic_statement>> loop{};
};







struct Typed_identifier
{
    Token const * identifier_token{nullptr};
    Type_info* type{nullptr};

    std::vector<Typed_identifier*> ids_with_same_type;
        // Used for inferring types.
        // When a type is inferred - go through this vector and verify that all ids has the same type.
};

struct Type_info
{
    virtual ~Type_info();
    virtual std::string get_type_id() const = 0;
};

struct Function_type : Type_info
{
    std::vector<Type_info*> in_parameters{};
    std::vector<Type_info*> out_parameters{};
    std::string get_type_id() const; // returns a mangled version of in and out parameters
};

struct Struct_type : Type_info
{
    Token const * struct_identifier_token{nullptr}; // cannot be null
    std::vector<std::unique_ptr<Typed_identifier>> members{}; // can be empty
    std::string get_type_id() const { ASSERT(struct_identifier_token != nullptr); return struct_identifier_token->token; }
};

struct Primitive_type : Type_info
{
    std::string type_name;
    std::string get_type_id() const { return type_name; }
};

struct Defined_type : Type_info
{
    Token const * identifier_token{nullptr}; // cannot be null;
    Type_info* type;
    std::string get_type_id() const { ASSERT(identifier_token != nullptr); return identifier_token->token; }
};


// Capture group:
// för tillfället: endast by value
// I det nya scopet skapas nya identifiers med de värden som motsvarande identifiers i parent scope har

struct Scope : Evaluated_value
{
    std::vector<std::unique_ptr<Typed_identifier>> identifiers;
    std::vector<std::unique_ptr<Type_info>> types;
    std::vector<Scope*> imported_scopes;
    virtual ~Scope();
};


struct Static_scope : Scope
{
    std::vector<std::unique_ptr<Static_statement>> statements;
};

struct Dynamic_scope : Scope
{
    std::vector<std::unique_ptr<Dynamic_statement>> statements;
};







struct Dependency
{
    virtual ~Dependency();
};



struct Type_of_identifier : Dependency
{
    Token const* identifier_name_token;
};
// ex:
// a := b; // a är beroende av typen av b


struct Type_dependancy : Dependency
{
    Token const* type_identifier_token;
};
// ex:
// a : S; // a är beroende av type identifier S.


struct Function_dependancy : Dependency
{
    Token const* function_identifier_token;
    unique_ptr<Rhs> arguments;
};






#endif