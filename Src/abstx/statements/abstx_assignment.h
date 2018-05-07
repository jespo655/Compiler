#pragma once

#include "abstx_statement.h"
#include "../expressions/variable_expression.h"
#include "../expressions/value_expression.h"

#include <sstream>

/*
Syntax:
a = 1;          // a gets the value 1
a = b;          // a gets the value of b.
a[0] = a[1];    // lhs can be any variable_expression, not only identifier.
a, b = 1, 2;    // more than one variable can be assigned at the same time. The evaluated count has to match. All types has to match.
a, b = foo();   // The lhs and rhs count will not match if a function in rhs returns more than one value.
*/

struct Abstx_assignment : Statement {

    Seq<Owned<Variable_expression>> lhs;
    Seq<Owned<Value_expression>> rhs;

    // lhs: []*Variable_expression;
    // rhs: []*Value_expression;

    std::string toS() const override {
        ASSERT(lhs.size > 0);
        ASSERT(rhs.size > 0);

        std::ostringstream oss;
        bool first = true;
        for (auto& var : lhs) {
            ASSERT(var != nullptr);
            if (!first) oss << ", ";
            oss << var->toS();
            first = false;
        }
        oss << " = ";
        first = true;
        for (auto& val : rhs) {
            ASSERT(val != nullptr);
            if (!first) oss << ", ";
            oss << val->toS();
            first = false;
        }
        oss << ";";
        return oss.str();
    }

    Parsing_status fully_parse() override; // implemented in statement_parser.cpp

    void generate_code(std::ostream& target) override {
        ASSERT(is_codegen_ready(status));
        ASSERT(lhs.size == rhs.size);
        for (int i = 0; i < lhs.size; ++i) {
            lhs[i]->generate_code(target); // this should be a valid c style lvalue
            target << " = ";
            lhs[i]->generate_code(target); // this should be a valid c style lvalue
            target << ";" << std::endl;
        }
        status = Parsing_status::CODE_GENERATED;
    };

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

