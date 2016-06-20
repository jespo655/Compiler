#ifndef abstx_h
#define abstx_h

#include <vector>
#include "lexer.h"
#include <memory> // std::unique_ptr
#include <string>
#include <map>


// Dynamic_statement: any statement that can be evaluated in a dynamic scope (function)
struct Dynamic_statement
{
    virtual ~Dynamic_statement() {}
    int dependencies = 0; // when 0 the statement can be resolved. Increment for each dependency added.
    Token const * context = nullptr;
};

// A Static_statement is a statement that can be evaluated in a static scope.
// Any static statement can also be performed in a dynamic context.
struct Static_statement : Dynamic_statement
{
    virtual ~Static_statement() {}
};




struct Type_info;
struct Typed_identifier;
struct Scope;
struct Function;


// Evaluated value: Anything that can hold a value.
struct Evaluated_value
{
    virtual ~Evaluated_value() {}
    virtual std::shared_ptr<Type_info> get_type() const { return nullptr; };
    Scope* local_scope;

    std::vector<Dynamic_statement*> dependant_statements; // used for type checker
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
    std::shared_ptr<Type_info> get_type() const;
};

// Value_list: A parenthesis with several comma-separated values
struct Value_list : Evaluated_value
{
    std::vector<std::unique_ptr<Evaluated_value>> values;
    std::shared_ptr<Type_info> get_type() const;
};

struct Identifier : Evaluated_variable
{
    Token const * identifier_token; // name & declaration context
    virtual ~Identifier() {}

    std::shared_ptr<Type_info> get_type() const;
};

struct Infix_op : Evaluated_value
{
    std::unique_ptr<Evaluated_value> lhs;
    Token const * op_token;
    std::unique_ptr<Evaluated_value> rhs;
    std::shared_ptr<Type_info> get_type() const;
    std::string get_mangled_op() const;
};

// function call: starts with "("
struct Function_call : Evaluated_variable, Dynamic_statement
{
    std::unique_ptr<Evaluated_value> function_identifier;
    std::vector<std::unique_ptr<Evaluated_value>> arguments; // these come in order
    std::map<std::string,std::unique_ptr<Evaluated_value>> named_arguments; // these comes last, but in any order

    std::shared_ptr<Type_info> get_type() const;
};

// getter: starts with "."
struct Getter : Evaluated_variable
{
    std::unique_ptr<Evaluated_value> struct_identifier;
    Token const * data_identifier_token;

    std::shared_ptr<Type_info> get_type() const;
};

// cast: starts with "_"
struct Cast : Evaluated_variable
{
    std::unique_ptr<Evaluated_value> casted_value;
    Token const * casted_type_token;
    std::shared_ptr<Type_info> get_type() const;
};

// array lookup: starts with "["
struct Array_lookup : Evaluated_variable
{
    std::unique_ptr<Evaluated_value> array_identifier;
    std::unique_ptr<Evaluated_value> position;
    std::shared_ptr<Type_info> get_type() const;
};




struct Type_info : Evaluated_value
{
    virtual ~Type_info() {}
    virtual std::string get_type_id() const = 0; // pure virtual
    std::shared_ptr<Type_info> get_type() const;
    bool operator==(const Type_info& o) const { return get_type_id() == o.get_type_id(); }
    bool operator!=(const Type_info& o) const { return !(*this == o); }
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
    // std::shared_ptr<Evaluated_value> value{nullptr};
    std::shared_ptr<Type_info> get_type() const { return type; }
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
    std::map<std::string,std::shared_ptr<Typed_identifier>> identifiers{}; // identifier name -> its type
    std::map<std::string,std::shared_ptr<Type_info>> types{}; // type name -> its value (the type it represents)
    std::map<std::string,std::shared_ptr<Function>> operators{};

    std::vector<Scope*> imported_scopes;
    virtual ~Scope() {}
    std::shared_ptr<Type_info> get_type() const;

    // returns nullptr if error (either not found or ambiguous call, is already logged)
    std::shared_ptr<Typed_identifier> get_identifier(const std::string& identifier_name, const Token_context& context);
    std::shared_ptr<Type_info> get_type(const std::string& type_name, const Token_context& context);
    std::shared_ptr<Function> get_operator(const std::string& mangled_op, const Token_context& context);

private:

    // returns true if error (either not found or ambiugous reference)
    // the first occurence found is returned as id/type
    bool get_identifier(const std::string& identifier_name, std::shared_ptr<Typed_identifier>& id, const Token_context& context, bool& ambiugous);
    std::shared_ptr<Type_info> get_type_no_checks(const std::string& type_name);
};


struct Static_scope : Scope, Static_statement
{
    std::vector<std::unique_ptr<Static_statement>> statements{};
};

struct Dynamic_scope : Scope, Dynamic_statement
{
    std::vector<std::unique_ptr<Dynamic_statement>> statements{};
    std::vector<std::unique_ptr<Dynamic_statement>> defer_statements{};
};



struct Function : Evaluated_value
{
    std::map<std::string,std::shared_ptr<Type_info>> in_parameters{};
    std::map<std::string,std::shared_ptr<Type_info>> out_parameters{}; // unnamed return values is called "__ret_value_1"

    std::map<std::string,std::unique_ptr<Evaluated_value>> default_in_values{};
    std::map<std::string,std::unique_ptr<Evaluated_value>> default_out_values{};

    std::unique_ptr<Dynamic_scope> body{nullptr};
    std::shared_ptr<Type_info> get_type() const;
};

struct Return_statement : Dynamic_statement
{
    std::map<std::string,std::unique_ptr<Evaluated_value>> return_values; // unnamed return values is called "__ret_value_1"
};

struct Using_statement : Static_statement
{
    std::unique_ptr<Evaluated_value> scope{nullptr};
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
    std::shared_ptr<Type_info> get_type() const;
};

struct For_clause : Dynamic_statement
{
    Token const * iterator_name_token{nullptr}; // might not exist
    std::unique_ptr<Evaluated_value> range{nullptr}; // has to exist
    std::unique_ptr<Evaluated_value> step{nullptr}; // might not exist
    std::unique_ptr<Dynamic_scope> loop{};
};





#endif