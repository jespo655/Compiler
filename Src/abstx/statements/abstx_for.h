#pragma once

#include "abstx_statement.h"
#include "../abstx_scope.h"
#include "../expressions/value_expression.h"
#include "../../utilities/unique_id.h"
#include "../../types/cb_range.h" // CB_Iterable

/*
for (n in range) {}
for (n in range, step=s) {}
for (n in range, index=s) {}
for (n in range, reverse) {}
*/

struct Abstx_for : Statement {

    Owned<Abstx_identifier> range = nullptr; // type has to be subclass of CB_Iterable
    Shared<const CB_Iterable> iterable_type = nullptr; // the type of range, put here for convenience
    bool reverse = false;
    uint64_t step = 1;
    bool anonymous_range = false; // if anonymous,

    Owned<Abstx_scope> scope;
    Shared<Abstx_identifier> it; // owned by the scope

    std::string toS() const override { return "for(){}"; }

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

    Parsing_status fully_parse() override; // implemented in statement_parser.cpp

    void generate_code(std::ostream& target) override {
        ASSERT(is_codegen_ready(status));

        if (anonymous_range) {
            // declare range
            target << "{ ";
            range->generate_code(target);
            target << " = ";
            range->value.generate_literal(target);
            target << "; ";
        }

        iterable_type->generate_for(target, range->name, it->name, step, reverse, anonymous_range);
        scope->generate_code(target);
        iterable_type->generate_for_after_scope(target);

        if (anonymous_range) {
            // close brace from before
            target << "}" << std::endl;
        }
    }

};


/*

// Generates c-code:

for (double n = range.start; n <= range.end; n+=step) {} // reverse = false
for (double n = range.end; n >= range.start; n-=step) {} // reverse = true

for (int index = 0; i <= (range.end-range.start)/step; i++) {} // step != 1
for (int index = 0; i <= (range.end-range.start); i++) {} // step = 1


*/
