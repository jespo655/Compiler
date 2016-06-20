#pragma once

#include "statement.h"

/*
for (n in range) {}
for (n in range, step=s) {}
for (n in range, index=s) {}
for (n in range, reverse) {}
*/

struct While_statement : Statement {

    std::string index_name;
    std::shared_ptr<Evaluated_value> range;
    bool reverse = false;
    double step;

    std::shared_ptr<Scope> scope;


    std::string toS() const override { return "while(){}"; }

    void debug_print(Debug_os& os, bool recursive=true) const override
    {
        os << "while(";
        if (recursive) {
            ASSERT(condition != nullptr);
            os << condition->toS();
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