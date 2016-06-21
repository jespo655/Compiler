#pragma once

#include "statement.h"
#include "identifier.h"
#include <sstream>
#include <vector>

/*
a : int = 1;    // declares a as the int 1
a := 1;         // infers the type from the value
a : int;        // infers the default value from the type
a := b;         // infers type from the variable b. The value of b overwrites the default value from the type of b.
*/

struct Declaration_statement : Statement {

    std::vector<std::shared_ptr<Identifier>> identifiers;
    std::vector<std::shared_ptr<Evaluated_value>> rhs;

    bool allow_in_static_scope() const override { return true; }
    bool allow_in_dynamic_scope() const override { return true; }

    std::string toS() const override {
        ASSERT(!identifiers.empty());
        std::ostringstream oss;
        bool first = true;
        bool all_typed = true;
        for (auto id : identifiers) {
            ASSERT(id != nullptr);
            ASSERT(!id->name.empty());
            if (!first) oss << ", ";
            oss << id->name;
            first = false;
            if (id->type == nullptr) all_typed = false;
        }
        if (all_typed) {
            oss << " : ";
            first = true;
            for (auto id : identifiers) {
                ASSERT(id != nullptr);
                ASSERT(id->type != nullptr);
                if (!first) oss << ", ";
                oss << id->type->toS();
                first = false;
            }
        } else {
            ASSERT(!rhs.empty());
        }
        if (!rhs.empty()) {
            if (all_typed) oss << " = ";
            else oss << " := ";
            first = true;
            for (auto ev : rhs) {
                ASSERT(ev != nullptr);
                oss << ev->toS();
            }
        }
        oss << ";";
        return oss.str();
    }
};


/*

a : int = 1;
a := 1;
a : int;
a := b;

// Generates c-code:

int a = 1;      // a : int = 1;         // trivial
int a = 1;      // a := 1;              // the type int is inferred from the literal 1
int a = 0;      // a : int;             // the value 0 is the default value of an int
T a = b;        // a := b;              // the type T is inferred from the identifier b

*/

