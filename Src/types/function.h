#pragma once

#include "type.h"
#include "primitives.h"
#include "string.h"
#include "dynamic_seq.h"
#include "pointers.h"

#include <string>

/*
Range - a compact data structure with a start and an end.
Can be iterated over

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
* in-variabler som matchar typens intyper
* ut-variabler som matchar typens uttyper
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


struct CB_Function_type : CB_Type
{
    static CB_Type type; // self reference / CB_Type
    CB_Dynamic_seq<CB_Sharing_pointer<CB_Type>> in_types;
    CB_Dynamic_seq<CB_Sharing_pointer<CB_Type>> out_types;

    operator CB_Type() { return *this; }

    std::string toS() const override {
        std::ostringstream oss;

        oss << "fn(";
        for (int i = 0; i < in_types.size; ++i) {
            if (i > 0) oss << ", ";
            oss << in_types[i].toS();
        }
        oss << ")";
        if (out_types.size == 0) return oss.str();

        oss << "->";
        if (out_types.size > 1) oss << "(";
        for (int i = 0; i < out_types.size; ++i) {
            if (i > 0) oss << ", ";
            oss << out_types[i].toS();
        }
        if (out_types.size > 1) oss << ")";
        return oss.str();
    }

    bool operator==(const CB_Function_type& o) const { return o.in_types == in_types && o.out_types == out_types; }
    bool operator!=(const CB_Function_type& o) const { return !(*this==o); }
    bool operator==(const CB_Type& o) const { toS() == o.toS(); }
    bool operator!=(const CB_Type& o) const { return !(*this==o); }


// Below: only useful for static tests, not for actual compiling
    template<typename... Types>
    void set_in_args() {
        in_types.clear();
        add_in_args<Types...>();
    }

    template<typename... Types>
    void set_out_args() {
        out_types.clear();
        add_out_args<Types...>();
    }
private:
    template<typename T, typename... Rest>
    void add_in_args() {
        in_types.add(CB_Sharing_pointer<CB_Type>(&T::type));
        add_in_args<Rest...>();
    }
    template<int i=0> add_in_args() {}

    template<typename T, typename... Rest>
    void add_out_args() {
        out_types.add(CB_Sharing_pointer<CB_Type>(&T::type));
        add_out_args<Rest...>();
    }
    template<int i=0> add_out_args() {}
};



struct CB_Function {

    static CB_Type type;
    CB_Sharing_pointer<CB_Function_type> fn_type;

    void (*v)() = nullptr; // function pointer

    std::string toS() const {
        return "function"; // FIXME: better toS()
    }

    // operator(): only useful for static tests, not for actual compiling
    template<typename... Types>
    void operator()(Types... args)
    {
        ASSERT(v != nullptr);
        ASSERT(fn_type != nullptr);
        assert_types<Types...>();
        auto fn_ptr = (void (*)(Types...))v;
        (*fn_ptr)(args...);
    }

    CB_Function() {}
    ~CB_Function() {}

    CB_Function& operator=(const CB_Function& fn) {
        v = fn.v;
        fn_type = fn.fn_type;
        // fn_type = alloc<Function_metadata>(*fn.fn_type);
        return *this;
    }
    CB_Function(const CB_Function& fn) { *this = fn; }

    CB_Function& operator=(nullptr_t np) {
        ASSERT(np == nullptr);
        v = nullptr;
        fn_type = CB_Function_type::type.default_value().get_shared<CB_Function_type>();
        // fn_type = alloc(Function_metadata());
        return *this;
    }
    CB_Function(const nullptr_t& fn) { *this = fn; }

private:

    template<int index>
    void assert_out_types() {
        ASSERT(index == fn_type->out_types.size, "The number of out arguments doesn't match the function type.");
    }

    template<int index, typename T, typename... Rest>
    void assert_out_types() {
        ASSERT(fn_type != nullptr);
        ASSERT(index < fn_type->out_types.size, "The number of out arguments doesn't match the function type.");
        const CB_Type& T1 = *fn_type->out_types[index];
        const CB_Type& T2 = get_type(T());
        ASSERT(T1 == T2, "Type mismatch at out type "+std::to_string(index)+", "+T1.toS()+" and "+T2.toS());
        return assert_out_types<index+1, Rest...>();
    }

    template<int index>
    void assert_in_types() {
        ASSERT(index == fn_type->in_types.size, "The number of in arguments doesn't match the function type.");
        ASSERT(0 == fn_type->out_types.size);
    }

    template<int index, typename T, typename... Rest>
    void assert_in_types() {
        ASSERT(fn_type != nullptr);
        if (index == fn_type->in_types.size) {
            return assert_out_types<0, T, Rest...>();
        }
        ASSERT(index < fn_type->in_types.size, "The number of in arguments doesn't match the function type.");
        const CB_Type& T1 = *fn_type->in_types[index];
        const CB_Type& T2 = get_type(T());
        ASSERT(T1 == T2, "Type mismatch at in type "+std::to_string(index)+", "+T1.toS()+" and "+T2.toS());
        return assert_in_types<index+1, Rest...>();
    }

    template<typename... Types>
    void assert_types() {
        if (sizeof...(Types) == 0) return;
        return assert_in_types<0, Types...>();
    }

    template<typename T, typename Type=CB_Type const*, Type=&T::type>
    const CB_Type& get_type(const T& object) { return T::type; }

    template<typename T, typename Type=CB_Type const*, Type=&T::type>
    const CB_Type& get_type(const T* pointer) { return T::type; }
};









