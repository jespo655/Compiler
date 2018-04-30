#pragma once

#include "cb_type.h"

#include "../utilities/sequence.h"
#include "../utilities/pointers.h"

#include <string>

/*
Syntax:
foo : fn(int, int)->(int, int);
sum : fn(int, int)->int;            // only one return value -> don't need the paren
bar : fn();                         // no return value -> don't need the arrow
*/






/*
CB_Function_type har:
* namm
* intyper
* uttyper

CB_Function har:
* type
* funktionspekare


Abstx Function har:
* CB_Function
* "osynliga" declaration statements med:
    in-variabler som matchar typens intyper
    ut-variabler som matchar typens uttyper
* function scope med statements



fn_scope ska kunna se alla in- och uttyper, plus alla saker i parent scope:

parent {
    // alla möjliga saker

    fn {
        // in-variabler
        // ut-variabler (referenser)
        // statements: automatgenererad initiering av variabler
        // * bas -> declaration statement med ev. default value (specat i fnen, annars default value för typen)
        // ??: hur ska värdena på argumenten stoppas in i funktionen?
        // * using -> using statements
        // * generic -> det är här den typen ändras

        fn_scope {
            // statements
        }
    }
}




//CB:
foo :: fn (a:int, b:int=2)->using c:$T {
    c = a+b;
}

// ska bli C:
void foo(int a, int b=2, T* c)
{
    *c = add_int_int(a,b); // funktionen add_int_int är automatgenererad från operator(int)+(int)->T
}

// CB-representation:
fn {
    // deklarationer används endast för typechecking

    // statements 0..n-1: (n = antalet in+utparametrar)
    a:int;
    b:int=2;
    c:*CB_Generic_type;

    // statements n..inf: using satements
    using c;

    fn_scope {
        // det scope som definierades i funktionsdefinitionen ovan
        // (fn_scope.parent = fn)
        c = a+b;
    }
}

// resolve order:
// 1) current scope
// 2) using scope
// 3) parent scope



Function_call_statement:

c = foo(a,b); // a,b,c kollas i ordning mot identifiers i statements 0..n-1
              // (senare tillägg): Ev. generiska typer resolvas och skapar därmed en kopia av funktionen med den nya typen klar
              // Alla inparametrar som inte har default values måste ha fått värden, annars error


// Default: const reference på alla icke-primitiva in-variabler, reference på alla ut-variabler





// specialfall där samma variabel används som både in-och utvariabel:
a = foo(a, a);
// -> måste skapa temporära kopior av a, för att använda som invariabler


*/



/*
@todo
WORK IN PROGRESS:
skriva om alla typer så att:
* de inte hanterar CB-data (all sådan hanteras av CB_Any)
* är subklasser av CB_Type och implementerar alla codegen-funktioner

varje instans av CB_Type/subklass av CB_Type är en egen typ
för typer som är statiska (int) - skapa "static CB_Type type" i klassen
    i konstruktorn; sätt uid = static_type.uid
för typer som kan vara olika (fn, struct) - gör ingenting; varje instans har ett eget uid
    kom ihåg att registrera typen på riktigt sen bara

alla typer måste ha ett defaultvärde
för funktion: titta om samma funktion redan är definierad som en typ i CB_Type::maps
    om inte - skapa en type med riktig constructor


 */



struct CB_Function : CB_Type
{
    static const bool primitive = true;
    static constexpr void(*_default_value)() = nullptr;
    seq<shared<const CB_Type>> in_types;
    seq<shared<const CB_Type>> out_types;

    std::string toS() const override {
        std::ostringstream oss;

        oss << "fn(";
        for (int i = 0; i < in_types.size; ++i) {
            if (i > 0) oss << ", ";
            oss << in_types[i]->toS();
        }
        oss << ")";
        if (out_types.size == 0) return oss.str();

        oss << "->";
        if (out_types.size > 1) oss << "(";
        for (int i = 0; i < out_types.size; ++i) {
            if (i > 0) oss << ", ";
            oss << out_types[i]->toS();
        }
        if (out_types.size > 1) oss << ")";
        return oss.str();
    }

    void finalize() {
        std::string tos = toS();
        for (const auto& tn_pair : typenames) {
            if (tn_pair.second == tos) {
                // found existing function type with the same signature -> grab its id
                uid = tn_pair.first;
                return;
            }
        }
        // no matching signature found -> register new type
        register_type(tos, sizeof(_default_value), &_default_value);
    }

    operator CB_Type() { return *this; }

    // code generation functions
    virtual ostream& generate_typedef(ostream& os) const {
        os << "typedef void(";
        generate_type(os);
        os << "*)(";
        // TODO: output arg types
        os << ");";
    }
    virtual ostream& generate_literal(ostream& os, void const* raw_data) const { ASSERT(raw_data == nullptr); return os << "null"; } // function literals cannot be generated from raw data; only null pointers
};








