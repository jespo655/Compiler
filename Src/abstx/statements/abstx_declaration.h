#pragma once

#include "abstx_statement.h"
#include "../expressions/abstx_identifier.h"
#include "../expressions/value_expression.h"

#include <sstream>

/*
Syntax:
a : int = 1;    // declares a as the int 1
a := 1;         // infers the type from the value
a : int;        // infers the default value from the type
a := b;         // infers type from the variable b. The value of b overwrites the default value from the type of b.
a, b := foo();  // The lhs and rhs count will not match if a function in rhs returns more than one value.
*/

struct Abstx_declaration : Statement {

    Seq<Owned<Abstx_identifier>> identifiers;
    Seq<Owned<Value_expression>> type_expressions;
    Seq<Owned<Value_expression>> value_expressions;

    std::string toS() const override {
        ASSERT(identifiers.size > 0);

        std::ostringstream oss;
        bool first = true;
        bool all_typed = true;
        for (auto& id : identifiers) {
            ASSERT(id != nullptr);
            ASSERT(id->name.length() > 0);
            if (!first) oss << ", ";
            oss << id->name;
            first = false;
            if (id->get_type() == nullptr) all_typed = false;
        }
        if (all_typed) {
            oss << " : ";
            first = true;
            for (auto& id : identifiers) {
                ASSERT(id != nullptr);
                ASSERT(id->get_type() != nullptr);
                if (!first) oss << ", ";
                oss << id->get_type()->toS();
                first = false;
            }
        } else {
            ASSERT(value_expressions.size > 0);
        }
        if (value_expressions.size > 0) {
            if (all_typed) oss << " = ";
            else oss << " := ";
            first = true;
            for (auto& ev : value_expressions) {
                ASSERT(ev != nullptr);
                oss << ev->toS();
            }
        }
        oss << ";";
        return oss.str();
    }

    Parsing_status fully_parse() override; // implemented in statement_parser.cpp

    void generate_code(std::ostream& target) override {
        ASSERT(is_codegen_ready(status));
        if (value_expressions.empty()) {
            // explicit uninitialized
            for (int i = 0; i < identifiers.size; ++i) {
                identifiers[i]->get_type()->generate_type(target);
                target << " ";
                identifiers[i]->generate_code(target); // this should be a variable name
                target << ";" << std::endl;
            }
        } else {
            ASSERT(identifiers.size == value_expressions.size);
            for (int i = 0; i < value_expressions.size; ++i) {
                identifiers[i]->get_type()->generate_type(target);
                target << " ";
                identifiers[i]->generate_code(target); // this should be a valid c style lvalue
                target << " = ";
                value_expressions[i]->generate_code(target); // this should be a valid c style lvalue
                target << ";" << std::endl;
            }
        }
        status = Parsing_status::CODE_GENERATED;
    };


};


/*

a : int = 1;
a := 1;
a : int;
a := b;
a, b := foo();

// Generates c-code:

int a = 1;      // a : int = 1;         // trivial
int a = 1;      // a := 1;              // the type int is inferred from the literal 1
int a = 0;      // a : int;             // the value 0 is the default value of an int
T a = b;        // a := b;              // the type T is inferred from the identifier b
T1 a; T2 b; foo(&a,&b); // a, b := foo();


*/

