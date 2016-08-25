#pragma once

#include "statement.h"
#include "scope.h"
#include "../expressions/value_expression.h"

/*
for (n in range) {}
for (n in range, step=s) {}
for (n in range, index=s) {}
for (n in range, reverse) {}
*/

struct For_statement : Statement {

    CB_String index_name;
    owned<Value_expression> range; // range or sequence
    CB_Bool reverse = false;
    CB_Float step;

    owned<CB_Scope> scope;

    std::string toS() const override { return "while(){}"; }

    void debug_print(Debug_os& os, bool recursive=true) const override
    {
        // FIXME: better for::toS()

        os << "For(";
        if (recursive) {
            ASSERT(range != nullptr);
            os << range->toS();
        }
        os << ") ";
        if (recursive) {
            ASSERT(scope != nullptr);
            scope->debug_print(os, recursive);
        }
        else os << std::endl;
    }

};


/*

// Generates c-code:

for (double n = range.start; n <= range.end; n+=step) {} // reverse = false
for (double n = range.end; n >= range.start; n-=step) {} // reverse = true

for (int index = 0; i <= (range.end-range.start)/step; i++) {} // step != 1
for (int index = 0; i <= (range.end-range.start); i++) {} // step = 1


*/