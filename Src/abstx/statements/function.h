#pragma once

#include "statement.h"
#include "scope.h"
#include "../types/function.h"

/*
Syntax:
foo : fn(int, int)->(int, int) = fn(a: int, b: int)->(c: int, d: int) { return a, b; };
sum : fn(int, int)->int = fn(a: int, b: int)->int { return a+b; }; // only one return value -> don't need the paren
bar : fn() = fn() {}; // no return value -> don't need the arrow
*/


/*

Teori:

Basic koncept:
    All typechecking är compile time -> vilka typer en funktion vill ha måste vara känt compile time
    Fråga: hur mycket är inbakat i typen och hur mycket i själva objektet?

    Objektet kan ändras under runtime, ej konstant




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




Att lägga till:


Statements:
    virtual void run(); // utför statementet i sin local scope

Value_expression:
    virtual Owned<Value_expression> eval(); // förenklar abstract syntax tree, bör returnera en så simpel trädstruktur som möjligt (helst value literal)
                                            // skapar alltid ett nytt objekt! (I värsta fall en djup kopia av sig själv)



*/




struct Function_arg
{
    shared<Identifier> identifier; // owned by the function scope
    CB_Any default_value;
    CB_Bool is_using = false; // only for structs - imports that struct's members into the function scope
    CB_Bool has_default_value = false;
    CB_Bool explicit_uninitialized = false;
}

struct Function : Value_expression
{
    shared<Function_type> function_type;
    CB_Dynamic_seq<CB_Owning_pointer<Function_arg>> in_args;
    CB_Dynamic_seq<CB_Owning_pointer<Function_arg>> out_args;

    virtual seq<owned<Value_expression>> eval() ; // TODO

    shared<CB_Type> get_type() override
    {
        return static_pointer_cast<CB_Type>(function_type);
    }

}
CB_Type Function::type = type("Function");





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
        function_scope.identifiers = std::map<CB_String, shared<Identifier>>();
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


