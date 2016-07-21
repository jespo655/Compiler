#pragma once

#include "statement.h"
#include "identifier.h"
#include "variable_expression.h"
#include "value_expression.h"
#include <sstream>
#include <vector>

/*
a = 1;          // a gets the value 1
a = b;          // a gets the value of b.
a[0] = a[1];    // lhs can be any evaluated variable, not only identifier.
a, b = 1, 2;    // more than one variable can be assigned at the same time. The evaluated count has to match. All types has to match.
*/

struct Assignment_statement : Statement {

    std::vector<std::shared_ptr<Variable_expression>> lhs;
    std::vector<std::shared_ptr<Value_expression>> rhs;

    std::string toS() const override {
        ASSERT(!lhs.empty());
        ASSERT(!rhs.empty());
        std::ostringstream oss;
        bool first = true;
        for (auto ev : lhs) {
            ASSERT(ev != nullptr);
            if (!first) oss << ", ";
            oss << ev->toS();
            first = false;
        }
        oss << " = ";
        first = true;
        for (auto ev : rhs) {
            ASSERT(ev != nullptr);
            if (!first) oss << ", ";
            oss << ev->toS();
            first = false;
        }
        oss << ";";
        return oss.str();
    }
};


/*

a = 1;
a = b;
a[0] = a[1];
a, b = 1, 2;
a, b = f(); // f has the type fn()->(int, int)


// Generates c-code:

a = 1;
a = b;
a[0] = a[1];
a = 1; b = 2;
tuple<int, int> __rval_N = f(); a = __rval_N[0]; b = __rval_N[1];

*/

