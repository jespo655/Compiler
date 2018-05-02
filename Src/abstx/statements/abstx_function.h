#pragma once

#include "abstx_statement.h"
#include "abstx_scope.h"
#include "../../types/cb_function.h"
#include "../../utilities/pointers.h"

/*
Syntax:
foo : fn(int, int)->(int, int) = fn(a: int, b: int)->(c: int, d: int) { return a, b; };
sum : fn(int, int)->int = fn(a: int, b: int)->int { return a+b; }; // only one return value -> don't need the paren
bar : fn() = fn() {}; // no return value -> don't need the arrow
*/

struct Abstx_function : Value_expression
{
    struct Function_arg
    {
        Shared<Identifier> identifier; // Owned by the function scope
        Any default_value;
        bool is_using = false; // only for structs - imports that struct's members into the function scope
        bool has_default_value = false;
        bool explicit_uninitialized = false;
    };

    Shared<Identifier> function_identifier; // contains the name and type of the function
    Seq<Owned<Function_arg>> in_args; // in arguments metadata
    Seq<Owned<Function_arg>> out_args; // out arguments metadata
    Owned<CB_Scope> scope; // function scope

    Shared<const CB_Type> get_type() override
    {
        return function_identifier->get_type();
    }

    Parsing_status finalize() override {
        if (is_codegen_ready(status)) return status;
        ASSERT(function_identifier != nullptr);

        // check function identifier
        if (!is_codegen_ready(function_identifier->status)) {
            if (is_error(function_identifier->status)) {
                // error in dependency -> this inherits the error
                status = function_identifier->status;
            } else {
                // dependency is just not finished parsing yet - wait for it and try again later
                // @todo add dependency chain
                status = Parsing_status::DEPENDENCIES_NEEDED;
            }
            return status;
        }

        // type is finalized -> get a pointer to it so we can typecheck arguments
        Shared<const CB_Function> fn_type = dynamic_pointer_cast<const CB_Function>(function_identifier->cb_type);
        ASSERT(fn_type != nullptr);

        // check arguments (the same way as function_identifier, but also check type)
        ASSERT(in_args.size == fn_type->in_types.size); // @todo this should maybe give a compile error / status TYPE_ERROR
        size_t index = 0;
        for (const auto& fa : in_args) {
            if (!is_codegen_ready(fa->identifier->status)) {
                if (is_error(fa->identifier->status)) {
                    status = fa->identifier->status;
                } else {
                    // @todo add dependency chain
                    status = Parsing_status::DEPENDENCIES_NEEDED;
                }
                return status;
            }
            if (*fa->identifier->cb_type != *fn_type->in_types[index]) {
                // @todo generate compile error "type mismatch in function type"
                status = Parsing_status::TYPE_ERROR;
                return status;
            }
            index++;
        }
        ASSERT(out_args.size == fn_type->out_types.size); // @todo this should maybe give a compile error / status TYPE_ERROR
        index = 0;
        for (const auto& fa : out_args) {
            if (!is_codegen_ready(fa->identifier->status)) {
                if (is_error(fa->identifier->status)) {
                    status = fa->identifier->status;
                } else {
                    // @todo add dependency chain
                    status = Parsing_status::DEPENDENCIES_NEEDED;
                }
                return status;
            }
            if (*fa->identifier->cb_type != *fn_type->out_types[index]) {
                // @todo generate compile error "type mismatch in function type"
                status = Parsing_status::TYPE_ERROR;
                return status;
            }
            index++;
        }

        // check function scope
        if (!is_codegen_ready(scope->finalize())) {
            status = scope->status;
            return status;
        }

        // we reached the end -> we are done
        status = Parsing_status::FULLY_RESOLVED;
        return status;
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
            arg->identifier->cb_type->generate_type(target);
            target << " ";
            if (arg->identifier->cb_type->is_primitive()) {
                target << "const* "; // pass primitives by value; non-primitives by const pointer
            }
            arg->identifier->generate_code(target);
        }
        if (in_args.size > 0 && out_args.size > 0) target << ", ";
        for (int i = 0; i < out_args.size; ++i) {
            const auto& arg = out_args[i];
            if (i) target << ", ";
            arg->identifier->cb_type->generate_type(target);
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

    CB_Scope function_scope;

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


