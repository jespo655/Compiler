#ifndef abstx_h
#define abstx_h

#include <vector>
#include "lexer.h"
#include <memory> // std::unique_ptr
#include <string>


// Dynamic_statement: any statement that can be evaluated in a dynamic scope (function)
struct Dynamic_statement
{
    virtual ~Dynamic_statement() {}
    int dependencies = 0; // when 0 the statement can be resolved. Increment for each dependency added.
};

// A Static_statement is a statement that can be evaluated in a static scope.
// Any static statement can also be performed in a dynamic context.
struct Static_statement : Dynamic_statement
{
    virtual ~Static_statement() {}
};




struct Type_info;
struct Typed_identifier;

// Evaluated value: Anything that can hold a value.
struct Evaluated_value
{
    virtual ~Evaluated_value() {}
    virtual std::shared_ptr<Type_info> get_type() { return nullptr; };
};

// Evaluated variable: Anything that can be assigned a value. I.E. not literals.
// Anything that can be assigned a value can also hold a value.
struct Evaluated_variable : Evaluated_value
{
    virtual ~Evaluated_variable() {}
};

struct Literal : Evaluated_value
{
    Token const * literal_token;
    virtual ~Literal() {}
    std::shared_ptr<Type_info> get_type();
};

// Value_list: A parenthesis with several comma-separated values
struct Value_list : Evaluated_value
{
    std::vector<std::unique_ptr<Evaluated_value>> values;
    std::shared_ptr<Type_info> get_type();
};

struct Identifier : Evaluated_variable
{
    Token const * identifier_token; // name & declaration context
    virtual ~Identifier() {}

    Typed_identifier* identity{nullptr};
    std::shared_ptr<Type_info> get_type();
};

struct Infix_op : Evaluated_value
{
    std::unique_ptr<Evaluated_value> lhs;
    Token const * op_token;
    std::unique_ptr<Evaluated_value> rhs;
    std::shared_ptr<Type_info> get_type();
};

// function call: starts with "("
struct Function_call : Evaluated_variable, Dynamic_statement
{
    std::unique_ptr<Evaluated_value> function_identifier;
    std::vector<std::unique_ptr<Evaluated_value>> arguments;

    std::shared_ptr<Type_info> get_type();
};

// getter: starts with "."
struct Getter : Evaluated_variable
{
    std::unique_ptr<Evaluated_value> struct_identifier;
    Token const * data_identifier_token;

    std::shared_ptr<Type_info> get_type();
};

// cast: starts with "_"
struct Cast : Evaluated_variable
{
    std::unique_ptr<Evaluated_value> casted_value;
    std::unique_ptr<Evaluated_value> casted_type; // has to evaluate to exactly one type identifier
    std::shared_ptr<Type_info> get_type();
};
//
// array lookup: starts with "["
struct Array_lookup : Evaluated_variable
{
    std::unique_ptr<Evaluated_value> array_identifier;
    std::unique_ptr<Evaluated_value> position;
    std::shared_ptr<Type_info> get_type();
};




struct Type_info : Evaluated_value
{
    virtual ~Type_info() {}
    virtual std::string get_type_id() const = 0;
    std::shared_ptr<Type_info> get_type();
};

struct Type_list : Type_info
{
    virtual ~Type_list() {}
    std::vector<std::shared_ptr<Type_info>> types;
    std::string get_type_id() const;
};

struct Typed_identifier : Identifier
{
    std::shared_ptr<Type_info> type{nullptr};
    std::shared_ptr<Type_info> get_type() { return type; }

    std::vector<Dynamic_statement*> dependant_statements;
        // Used for inferring types.
        // When a type is inferred - go through this vector try to resolve the statements.
};


// Function_type: the type of foo := fn(){}
struct Function_type : Type_info
{
    std::vector<std::shared_ptr<Type_info>> in_parameters{};
    std::vector<std::shared_ptr<Type_info>> out_parameters{};
    std::string get_type_id() const; // returns a mangled version of in and out parameters
};

// Struct_type: the type of s : S; , when S := struct {};
struct Struct_type : Type_info
{
    std::vector<std::unique_ptr<Typed_identifier>> members{}; // can be empty
    std::string get_type_id() const; // returns a mangled version of members
};

// Primitive_type: the type of a : int;
// Includes the type "type", which is the type of int2 : type = int;, S := struct{}; och f := fn();
struct Primitive_type : Type_info
{
    std::string type_name{};
    int size_bytes = -1; // -1 if unknown
    std::string get_type_id() const { return type_name; }
    Primitive_type() {}
    Primitive_type(std::string s, int size = -1) : type_name{s}, size_bytes{size} {}
};

// Defined_type: the type int2 := int; Has the same properties as int, but cannot be implicitly casted back and forth
// struct Defined_type : Type_info
// {
//     Token const * identifier_token{nullptr}; // cannot be null;
//     Type_info* type{nullptr};
//     std::string get_type_id() const { return identifier_token->token; }
// };

// Unresolved_type: the type of s : S; , where S is not yet a known identifier
struct Unresolved_type : Type_info
{
    Token const * identifier_token{nullptr}; // cannot be null;
    std::string get_type_id() const { return identifier_token->token; }
};

struct Array_type : Type_info
{
    std::shared_ptr<Type_info> type{nullptr}; // cannot be null;
    int size = 0;
    bool dynamic = false;
    std::string get_type_id() const { return type->get_type_id() + "[" + (dynamic?"..":std::to_string(size)) + "]"; }
};







// Capture group:
// för tillfället: endast by value
// I det nya scopet skapas nya identifiers med de värden som motsvarande identifiers i parent scope har

struct Scope : Evaluated_value
{
    std::vector<std::shared_ptr<Typed_identifier>> identifiers{};
    // std::vector<std::unique_ptr<Type_info>> types; // maybe remove
    std::vector<Scope*> imported_scopes;
    virtual ~Scope() {}
    std::shared_ptr<Type_info> get_type();
};


struct Static_scope : Scope, Static_statement
{
    std::vector<std::unique_ptr<Static_statement>> statements{};
    // std::vector<Dependency> dependencies; // should be moved to Typed_variable
};

struct Dynamic_scope : Scope, Dynamic_statement
{
    std::vector<std::unique_ptr<Dynamic_statement>> statements{};
    // std::vector<std::unique_ptr<Dynamic_statement>> defer_statements; // TODO
    // dynamic scopes cannot have dependencies -
    //   everything must be known the moment they are used.
    // If a statement cannot be resolved, then set up a dependency in all imported static scopes.
};



struct Function : Evaluated_value
{
    std::unique_ptr<Function_type> type{nullptr};
    std::vector<Token const *> in_parameter_name_tokens{}; // must be of the same length as type.in_parameters
    std::vector<Token const *> out_parameter_name_tokens{}; // must be of the same length as type.out_parameters. Name can be empty
    std::unique_ptr<Dynamic_scope> body{nullptr};
    std::shared_ptr<Type_info> get_type();
};

struct Return_statement : Dynamic_statement
{
    std::vector<std::unique_ptr<Evaluated_value>> return_values;
};

struct Using_statement : Static_statement
{
    std::unique_ptr<Evaluated_variable> scope{nullptr};
};

// Declaration: any statement including ":" and maybe "="
struct Declaration : Static_statement
{
    std::vector<std::vector<std::shared_ptr<Typed_identifier>>> lhs{}; // these are stored in the local scope
    std::vector<std::unique_ptr<Evaluated_value>> rhs{}; // can be empty . One part can be a function that returns several values. Check that the count matches when all top-level functions are resolved.
};

// Assignment: any assignment not including ":"
struct Assignment : Dynamic_statement
{
    std::vector<std::unique_ptr<Evaluated_variable>> lhs{};
    Token const * assignment_op_token{nullptr};
    std::vector<std::unique_ptr<Evaluated_value>> rhs{};
};


struct Elsif {
    std::unique_ptr<Evaluated_value> condition{nullptr}; // has to evaluate to exactly one bool
    std::unique_ptr<Dynamic_scope> body{};
};

// if-elsif-else-then
struct If_clause : Dynamic_statement
{
    std::unique_ptr<Evaluated_value> condition{nullptr}; // has to evaluate to exactly one bool
    std::unique_ptr<Dynamic_scope> if_true{};
    std::vector<std::unique_ptr<Elsif>> elsifs{};
    std::unique_ptr<Dynamic_scope> if_false{};
    std::unique_ptr<Dynamic_scope> then_body{nullptr};
};

struct While_clause : Dynamic_statement
{
    std::unique_ptr<Evaluated_value> condition{nullptr};
    std::unique_ptr<Dynamic_scope> loop{};
};




struct Range : Evaluated_value
{
    std::unique_ptr<Evaluated_value> start{nullptr};
    std::unique_ptr<Evaluated_value> end{nullptr};
    std::shared_ptr<Type_info> get_type();
};

struct For_clause : Dynamic_statement
{
    Token const * iterator_name_token{nullptr}; // might not exist
    std::unique_ptr<Evaluated_value> range{nullptr}; // has to exist
    std::unique_ptr<Evaluated_value> step{nullptr}; // might not exist
    std::unique_ptr<Dynamic_scope> loop{};
};





#endif