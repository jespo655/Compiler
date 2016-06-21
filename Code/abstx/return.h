#pragma once

#include "statement.h"
#include "evaluated_value.h"

#include <vector>

/*
return 1;           // returns the int 1
return 1, 2;        // returns 2 ints: 1, 2
return a=1, b=2;    // assignes 1 and 2 to the return variables a and b
return b=2, a=1;    // the same as above. Order of arguments doesn't matter.
return 1, b=2;      // named and non-named return values can be mixed. However, all named values has to come last.
b=2; return 1;      // the return count only has to include all non-named return values.
*/

struct Return_statement : Statement {

    std::vector<std::shared_ptr<Evaluated_value>> return_parameters;

    bool allow_in_static_scope() const override { return false; }
    bool allow_in_dynamic_scope() const override { return true; }

    std::string toS() const override {
        return "return statement";
        // FIXME: better return::toS()
    }
};


/*

{
    return 1;
    return 1, 2;
    return a=1, b=2;
    return b=2, a=1;
    return 1, b=2;
    b=2; return 1;
}

// Generates c-code:

{
    __rpar_1 = 1; return __rpar_1;                                  // return 1;
    __rpar_1 = 1, __rpar_2 = 2; return tuple(__rpar_1, __rpar_2);   // return 1, 2;
    a = 1, b = 2; return tuple(a, b);                               // return a=1, b=2;
    b = 2, a = 1; return tuple(a, b);                               // return b=2, a=1;
    __rpar_1 = 1: b = 2; return tuple(__rpar_1, b);                 // return 1, b=2;
    b = 2; __rpar_1 = 1; return tuple(__rpar_1, b);                 // b=2; return 1;
}

*/

