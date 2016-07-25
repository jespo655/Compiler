#pragma once

#include "statement.h"
#include "value_expression.h"
#include "scope.h"

/*
for (n in range) {}
for (n in range, step=s) {}
for (n in range, index=s) {}
for (n in range, reverse) {}
*/

struct For_statement : Statement {

    std::string index_name;
    std::shared_ptr<Value_expression> range;
    bool reverse = false;
    double step;

    std::shared_ptr<Scope> scope;

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