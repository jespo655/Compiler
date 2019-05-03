#pragma once

#include "value_expression.h"
#include "abstx_identifier.h"
#include "../abstx_scope.h"
#include "../../types/cb_function.h"

/*
Syntax:
foo : fn(int, int)->(int, int) = fn(a: int, b: int)->(c: int, d: int) { return a, b; };
sum : fn(int, int)->int = fn(a: int, b: int)->int { return a+b; }; // only one return value -> don't need the paren
bar : fn() = fn() {}; // no return value -> don't need the arrow
*/

struct Abstx_function_literal : Value_expression
{
    struct Function_arg
    {
        Shared<Abstx_identifier> identifier; // Owned by the function scope
        bool is_using = false; // only for structs - imports that struct's members into the function scope
        bool has_default_value = false;
        bool explicit_uninitialized = false;
        bool generic_id = false; // $ marker on the identifier -> value must be known at compile time of function call
        bool generic_type = false; // $ marker on the type -> type must be known at compile time of function call
    };

    Abstx_identifier function_identifier; // contains the hidden name and type of the function
    Seq<Function_arg> in_args; // in arguments metadata
    Seq<Function_arg> out_args; // out arguments metadata
    Abstx_function_scope scope; // function scope
    bool code_generated = false;

    std::string toS() const override;

    // the abstract identifier should be owned by the function scope
    void add_arg(bool in, Shared<Abstx_identifier> id);

    Shared<const CB_Type> get_type() override;
    bool has_constant_value() const;
    const Any& get_constant_value() override;

    void generate_code(std::ostream& target) const override;
    // all declaration must be generated in a global scope
    // before this, all types must be defined
    void generate_declaration(std::ostream& target, std::ostream& header);

    void finalize() override; // implemented in expression_parser.cpp

private:
    void generate_declaration_internal(std::ostream& target, bool header) const;
};



/*

S :: struct {}

f1 :: fn() {};
f2 :: fn(S) {};
f3 :: fn(S)->S {};


I :: interface(T)
{
    foo1 : fn(T) {};
    foo2 : fn(T)->T {};
    // foo3 : fn() {};
}

foo :: fn(I)
{
    foo1(I);
    foo2(I);
}

// Interface funkar som en generisk variabel
// Skapar en mapping från interface methods till funktionsimplementationer med rätt funktionssignatur
// Denna mapping kan göras compile time så länge intypen är känd

*/































/*

TODO: add flag "compile_time_only" which is true if any compile_time_only thing is accessed.
Log error if compile time only function is called runtime.

*/

/*
struct Function_instance {
    static CB_Type type; // Type "Function_pointer" - only here to conform to standard for a CB value. Special cases are needed elsewhere, to use function_type instead for type checking.
    Function_type function_type; // holds all metadata

    Abstx_scope function_scope;

    void reset_function_scope() {
        // reset identifiers and imported scopes
        function_scope.identifiers = std::map<CB_String, Shared<Identifier>>();
        function_scope.imported_scopes.clear();
        ASSERT(function_scope.using_statements.size == 0); // should not be used for dynamic scopes

        // now add the in and out args again, with default values
        for (const Function_arg& arg : function_type.metadata.in_args) {
            ASSERT(arg.id.size > 0);
            ASSERT((default_value.v_type == Struct_instance::type && default_value.value<Struct_instance>().struct_type == arg.type)
                || (default_value.v_type == Function_instance::type && default_value.value<Function_instance>().function_type == arg.type)
                || (default_value.v_type != Struct_instance::type && default_value.v_type != Function_instance::type) && default_value.v_type == arg.type);
            // function_scope.identifiers[arg.id] = Identifier(arg.id, arg.default_value);

            // Identifier måste ha en owner
            // Id.owner = declaration statement
            // i function
        }
    }

    bool check_types(
        const CB_Dynamic_seq<CB_Any>& in_args,
        std::map<CB_String, CB_Any> named_in_args,
        const CB_Dynamic_seq<CB_Sharing_pointer<CB_Any>>& out_args)
    {

    }


    CB_Dynamic_seq<CB_Any> operator()(
        const CB_Dynamic_seq<CB_Any>& in_args,
        std::map<CB_String, CB_Any> named_in_args,
        const CB_Dynamic_seq<CB_Sharing_pointer<CB_Any>>& out_args)
    {
        reset_function_scope();
        bool types_ok = check_types(in_args, named_in_args, out_args);
    }



*/















/*
S1 := struct {
    a : int;
}

S2 := struct {
    using s : S1;
    b : int;
}

foo := fn(s : S1) {}

s1 : S1;
s2 : S2;
foo(s1);
foo(s2.s);
s2.s.a;
*/


