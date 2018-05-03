#pragma once

#include "abstx_statement.h"
#include "../abstx_scope.h"
#include "../expressions/value_expression.h"
#include "../../utilities/unique_id.h"

#include "cb_range.h"
#include "unique_id.h"

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

    Parsing_status finalize() override {
        if (is_codegen_ready(status)) return status;

        // range
        ASSERT(range != nullptr);
        if (anonymous_range) {
            range->name = "_range_" + std::to_string(get_unique_id());
        }
        if (!is_codegen_ready(range->finalize())) {
            status = range->status;
            return status;
        }

        // scope
        ASSERT(scope != nullptr);
        if (!is_codegen_ready(scope->finalize())) {
            status = scope->status;
            return status;
        }

        // it
        ASSERT(it != nullptr);
        if (!is_codegen_ready(it->status)) {
            if (is_error(it->status)) status = it->status;
            else status = Parsing_status::DEPENDENCIES_NEEDED;
            return status;
        }

        status = Parsing_status::FULLY_RESOLVED;
        return status;
    }

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
