#pragma once

#include "abstx_statement.h"
#include "../expressions/abstx_identifier.h"

/*
for (n in range) {}
for (n in range, step=s) {}
for (n in range, index=s) {}
for (n in range, reverse) {}
*/

namespace Cube {

struct CB_Iterable;
struct Abstx_scope;

struct Abstx_for : Statement {

    Owned<Abstx_identifier> range = nullptr; // type has to be subclass of CB_Iterable
    Shared<const CB_Iterable> iterable_type = nullptr; // the type of range, put here for convenience
    bool reverse = false;
    uint64_t step = 1;
    bool anonymous_range = false; // if anonymous,

    Owned<Abstx_scope> scope;
    Shared<Abstx_identifier> it; // owned by the scope

    std::string toS() const override;

    void debug_print(Debug_os& os, bool recursive=true) const override;

    Parsing_status fully_parse() override; // implemented in statement_parser.cpp

    void generate_code(std::ostream& target) const override;

};

}

/*

// Generates c-code:

for (double n = range.start; n <= range.end; n+=step) {} // reverse = false
for (double n = range.end; n >= range.start; n-=step) {} // reverse = true

for (int index = 0; i <= (range.end-range.start)/step; i++) {} // step != 1
for (int index = 0; i <= (range.end-range.start); i++) {} // step = 1


*/
