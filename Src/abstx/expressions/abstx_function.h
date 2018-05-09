#pragma once

#include "value_expression.h"
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
        Any default_value;
        bool is_using = false; // only for structs - imports that struct's members into the function scope
        bool has_default_value = false;
        bool explicit_uninitialized = false;
    };

    Shared<Abstx_identifier> function_identifier; // contains the name and type of the function
    Seq<Owned<Function_arg>> in_args; // in arguments metadata
    Seq<Owned<Function_arg>> out_args; // out arguments metadata
    Owned<Abstx_function_scope> scope; // function scope

    std::string toS() const override {
        // @todo: write better toS()
        return "function";
    }

    // the abstract identifier should be owned by the function scope
    void add_arg(bool in, Shared<Abstx_identifier> id) {
        Owned<Function_arg> arg = alloc(Function_arg());
        arg->identifier = id;
        arg->default_value = id->get_type()->default_value();
        if (in) in_args.add(std::move(arg));
        else out_args.add(std::move(arg));
    }

    Shared<const CB_Type> get_type() override
    {
        return function_identifier->get_type();
    }

    bool has_constant_value() const {
        return false; // @todo: check if this is reasonable
    }

    const Any& get_constant_value() override {
        static Any no_value;
        return no_value;
    }

    void generate_code(std::ostream& target) override {
        ASSERT(is_codegen_ready(status));

        // function declaration syntax
        target << "void "; // all cb functions returns void
        function_identifier->generate_code(target);
        target << "(";
        for (int i = 0; i < in_args.size; ++i) {
            const auto& arg = in_args[i];
            if (i) target << ", ";
            arg->identifier->get_type()->generate_type(target);
            target << " ";
            if (!arg->identifier->get_type()->is_primitive()) {
                target << "const* "; // pass primitives by value; non-primitives by const pointer
            }
            arg->identifier->generate_code(target);
        }
        if (in_args.size > 0 && out_args.size > 0) target << ", ";
        for (int i = 0; i < out_args.size; ++i) {
            const auto& arg = out_args[i];
            if (i) target << ", ";
            arg->identifier->get_type()->generate_type(target);
            target << "* "; // always pass non-cost pointer to the original value
            arg->identifier->generate_code(target);
        }
        target << ") ";
        scope->generate_code(target);
        status = Parsing_status::CODE_GENERATED;

        // Generated code:
        // int is primitive, T is not primitive
        // default values for a, b are set during the function call (another abstx node)
        /*
        void foo(int a, T const* b, int* a, T* b) {
            // scope satatements
        }
        */
    };

    void finalize() override {
        if (is_error(status) || is_codegen_ready(status)) return;
        // finalize all identifiers

        function_identifier->finalize();
        if (is_error(function_identifier->status)) {
            status = function_identifier->status;
            return;
        } else if (function_identifier->status == Parsing_status::DEPENDENCIES_NEEDED) {
            status = Parsing_status::DEPENDENCIES_NEEDED;
        }

        for (const auto& arg : in_args) {
            arg->identifier->finalize();
            if (is_error(arg->identifier->status)) {
                status = arg->identifier->status;
                return;
            } else if (arg->identifier->status == Parsing_status::DEPENDENCIES_NEEDED) {
                status = Parsing_status::DEPENDENCIES_NEEDED;
            }
        }

        for (const auto& arg : out_args) {
            arg->identifier->finalize();
            if (is_error(arg->identifier->status)) {
                status = arg->identifier->status;
                return;
            } else if (arg->identifier->status == Parsing_status::DEPENDENCIES_NEEDED) {
                status = Parsing_status::DEPENDENCIES_NEEDED;
            }
        }

        // note: function scope doesn't need to be fully parsed for the function literal to be

        // update status
        if (!is_error(status) && status != Parsing_status::DEPENDENCIES_NEEDED) status = Parsing_status::FULLY_RESOLVED;
    }
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


