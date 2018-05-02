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
    Seq<Owned<Value_expression>> rhs;

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
            if (id->cb_type == nullptr) all_typed = false;
        }
        if (all_typed) {
            oss << " : ";
            first = true;
            for (auto& id : identifiers) {
                ASSERT(id != nullptr);
                ASSERT(id->cb_type != nullptr);
                if (!first) oss << ", ";
                oss << id->cb_type->toS();
                first = false;
            }
        } else {
            ASSERT(rhs.size > 0);
        }
        if (rhs.size > 0) {
            if (all_typed) oss << " = ";
            else oss << " := ";
            first = true;
            for (auto& ev : rhs) {
                ASSERT(ev != nullptr);
                oss << ev->toS();
            }
        }
        oss << ";";
        return oss.str();
    }

    Parsing_status finalize() override {
        if (is_codegen_ready(status)) return status;

        if (rhs.size != 0 && identifiers.size != rhs.size) {
            status = Parsing_status::TYPE_ERROR;
            return status;
        }
        for (const auto& id : identifiers) {
            ASSERT(id != nullptr)
            if (!is_codegen_ready(id->finalize())) {
                status = id->status; // @todo: save all dependencies in a list for later (maybe)
                return status;
            }
        }
        for (const auto& val_exp : rhs) {
            ASSERT(val_exp != nullptr)
            if (!is_codegen_ready(val_exp->finalize())) {
                status = val_exp->status;
                return status;
            }
        }
        // we reached the end -> we are done
        status = Parsing_status::FULLY_RESOLVED;
        return status;
    }

    void generate_code(std::ostream& target) override {
        ASSERT(is_codegen_ready(status));
        if (rhs.empty()) {
            // explicit uninitialized
            for (int i = 0; i < identifiers.size; ++i) {
                identifiers[i]->cb_type->generate_type(target);
                target << " ";
                identifiers[i]->generate_code(target); // this should be a variable name
                target << ";" << std::endl;
            }
        } else {
            ASSERT(identifiers.size == rhs.size);
            for (int i = 0; i < rhs.size; ++i) {
                identifiers[i]->cb_type->generate_type(target);
                target << " ";
                identifiers[i]->generate_code(target); // this should be a valid c style lvalue
                target << " = ";
                rhs[i]->generate_code(target); // this should be a valid c style lvalue
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

