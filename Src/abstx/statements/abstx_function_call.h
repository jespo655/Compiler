#pragma once

#include "abstx_statement.h"
#include "../expressions/abstx_function.h"

/*

In compiled CB, all functions returns void - return values are instead handled by pointer references that are included as arguments.
Therefore, we can't use CB functions as value expressions. Instead, any function call has to be evaluated first and the
  return values stored in temporary values. Then those temporary values can be used as value expressions instead.

Example:
    foo := fn(i:int)->r:int { return i; }
    a = foo(foo(2));
generates C code:
    void foo(int i, int* r) { *r = i; return; }
    { // temporary values are kept inside a separate scope
        int _tmp_123;
        foo(2, &_tmp_123);
        foo(_tmp_123, &a);
    }




Syntax:
    foo(); // all return values are ignored
    increment i; // increment is a prefix operator which modifies i
    i add 2; // add is an infix operator which modifies i


    Enklaste: typen innehåller all typinformation. Fn_Type: fn(int)->int
    f : fn(int)->int = fn(a:int)->int{return a;}
    a := f(2); // ok
    a := f("2"); // ej ok: type mismatch: expected int but found string
    a := f(); // ej ok: wrong number of arguments: expected 1 argument but found 0

    Tillägg: default arguments.
    f : fn(int)->int = fn(a:int=2)->int{return a;}
    a := f(2); // ok
    a := f("2"); // ej ok, type mismatch: expected int but found string
    a := f(); // ok, a får värdet 2 (givet i funktionsobjektet) <-- OBS! Funktionsobjektet gör detta möjligt, inte typen

    Tillägg: named arguments.
    f : fn(int,int)->int = fn(a:int,b:int)->int{return a;}
    a := f(1,2); // ok, returnerar 1
    a := f(1,b=2); // ok, returnerar 1
    a := f(1,a=1); // ej ok, a får värde 2 gånger och b får inget värde
    a := f(b=2,a=1); // ok, returnerar 1


    Default och named arguments är egenskaper hos funktionsobjektet, inte typen.
    Lösning: Tillåt endast för statiska objekt - funktionspekare som är garanterade att aldrig ändras.

*/

struct Abstx_function_call : Statement
{
    Shared<Abstx_function_literal> function = nullptr; // inferred through function_pointer in fully_parse
    Owned<Variable_expression> function_pointer;
    Seq<Owned<Value_expression>> in_args;
    Seq<Shared<Variable_expression>> out_args;

    std::string toS() const override { return "function call statement"; }

    Parsing_status fully_parse() override; // implemented in statement_parser.cpp

    void generate_code(std::ostream& target) const override
    {
        ASSERT(is_codegen_ready(status));
        function_pointer->generate_code(target);
        target << "(";
        for (int i = 0; i < in_args.size; ++i) {
            const auto& arg = in_args[i];
            if (i) target << ", ";
            if (!arg->get_type()->is_primitive()) {
                target << "&"; // pass primitives by value; non-primitives by const pointer
            }
            arg->generate_code(target);
        }
        if (in_args.size > 0 && out_args.size > 0) target << ", ";
        for (int i = 0; i < out_args.size; ++i) {
            const auto& arg = out_args[i];
            if (i) target << ", ";
            target << "&"; // always pass non-cost pointer to the original value
            arg->generate_code(target);
        }
        target << ");" << std::endl;

    }
};


struct Abstx_function_call_expression : Variable_expression {
    Shared<Abstx_function_call> function_call;
    std::string toS() const override { return "function call expression"; }

    Shared<const CB_Type> get_type() override {
        ASSERT(false, "Abstx_function_call_expression::get_type() not allowed since functions can have several types - check funcion_call->out_args instead");
    }

    void generate_code(std::ostream& target) const override {
        ASSERT(function_call);
        function_call->generate_code(target);
    }

    bool has_constant_value() const override {
        // @todo: function call has constant value if all arguments are consants, and if the function scope is self contained (no bi effects)
        return false;
    }

    const Any& get_constant_value() override {
        static const Any no_value;
        return no_value;
    }

    void finalize() override {
        // the function call should be finalized by the scope finalizer
        ASSERT(function_call != nullptr);
        status = function_call->status;
    }
};
