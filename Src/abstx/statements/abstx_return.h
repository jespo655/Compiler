#pragma once

#include "abstx_statement.h"

/*
return 1;           // returns the int 1
return 1, 2;        // returns 2 ints: 1, 2
return a=1, b=2;    // assignes 1 and 2 to the return variables a and b
return b=2, a=1;    // the same as above. Order of arguments doesn't matter.
return 1, b=2;      // named and non-named return values can be mixed. However, all named values has to come last.
b=2; return 1;      // the return count only has to include all non-named return values.
*/

struct Return_statement : Statement {

    // Return parameters should not be included in this statement.
    // Instead, during parsing additional assignment statements should be inserted directly
    //   before the return statement.

    std::string toS() const override {
        return "return statement";
    }

    Parsing_status finalize() override {
        if (is_codegen_ready(status)) return status;

        // we reached the end -> we are done
        status = Parsing_status::FULLY_RESOLVED;
        return status;
    }

    void generate_code(std::ostream& target) override {
        ASSERT(is_codegen_ready(status));
        target << "return;";
        status = Parsing_status::CODE_GENERATED;
    };
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
    // Note: deprecated codegen - tupes doesn't exist in C

    _cb_rpar_1 = 1; return _cb_rpar_1;                                  // return 1;
    _cb_rpar_1 = 1, _cb_rpar_2 = 2; return tuple(_cb_rpar_1, _cb_rpar_2);   // return 1, 2;
    a = 1, b = 2; return tuple(a, b);                                   // return a=1, b=2;
    b = 2, a = 1; return tuple(a, b);                                   // return b=2, a=1;
    _cb_rpar_1 = 1: b = 2; return tuple(_cb_rpar_1, b);                 // return 1, b=2;
    b = 2; _cb_rpar_1 = 1; return tuple(_cb_rpar_1, b);                 // b=2; return 1;
}

*/

