#pragma once


#include "type.h"
#include "function.h"
#include "dynamic_seq.h"
#include "pointers.h"

#include <string>

/*
A generic function is a function with one or more generic arguments.
For each set of types that the function is called with, a new function instance is created and compiled.
This means that the generic function is closely connected to the parser.

Syntax:
foo : fn($T, T)->T; // T is decided by the first in argument
foo : fn(T, $T)->T; // T is decided by the second in argument
// foo : fn(T); // error, unable to infer the type T; no type decider
// foo : fn(T)->$T; // error, unable to infer the type T; type deciders has to be in the in argument list
foo : fn($T1, $T2); // more than one type can be generic
foo : fn(int, $T); // generic and non-generic types can be mixed
*/



typedef std::map<CB_Int, CB_Type> Type_map; // from generic index to inferred type

// FIXME: move these to parser
struct Abstx_Generic_function {};
CB_Function recompile(CB_Sharing_pointer<Abstx_Generic_function> abstx, const Type_map& inferred_map) {} // FIXME: move to parser files









// inherits id, type and has_default_value from Function_arg
struct Generic_arg : Function_arg {
    CB_Int generic_index = -1; // -1: not generic
    CB_Bool deciding = false;
};

struct Generic_function_metadata {
    CB_Dynamic_seq<Generic_arg> in_args;
    CB_Dynamic_seq<Generic_arg> out_args;
};

struct CB_Generic_function
{
    static CB_Type type;
    CB_Owning_pointer<Generic_function_metadata> metadata;
    CB_Dynamic_seq<CB_Function> constructed_fns;
    CB_Sharing_pointer<Abstx_Generic_function> abstx; // only compile time

    std::string toS() {
        return "generic_function"; // FIXME: better toS()
    }

    CB_Generic_function(CB_Sharing_pointer<Abstx_Generic_function> abstx) : abstx{abstx} {}
    ~CB_Generic_function() {}

    CB_Generic_function& operator=(const CB_Generic_function& fn) {
        metadata = alloc<Generic_function_metadata>(*fn.metadata);
        constructed_fns = fn.constructed_fns;
        abstx = fn.abstx;
        return *this;
    }
    CB_Generic_function(const CB_Generic_function& fn) { *this = fn; }

    CB_Generic_function& operator=(CB_Generic_function&& fn) {
        constructed_fns = std::move(fn.constructed_fns);
        metadata = std::move(fn.metadata);
        abstx = fn.abstx;
        return *this;
    }
    CB_Generic_function(CB_Generic_function&& fn) { *this = fn; }

    void add_specialization(const CB_Function& fn) {
        // verify types
        ASSERT(verify_types(fn.metadata->in_args));
        constructed_fns.add(fn);
    }

    CB_Function get_specialization(const CB_Dynamic_seq<Function_arg>& in_args) {
        // infer types
        Type_map inferred_map = infer_types(in_args);
        CB_Function fn = recompile(abstx, inferred_map);
        constructed_fns.add(fn);
        return fn;
    }

    Type_map infer_types(const CB_Dynamic_seq<Function_arg>& in_args) {
        ASSERT(false, "NYI");
        return Type_map();
    }

    bool verify_types(const CB_Dynamic_seq<Function_arg>& in_args) {
        ASSERT(false, "NYI");
        return false;
    }

};
CB_Type CB_Generic_function::type = CB_Type("generic_fn");

