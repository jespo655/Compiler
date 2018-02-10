#pragma once

#include <fstream>
// #include "../abstx/abstx.h"
// #include "../abstx/statements/function.h"

#include <set>
#include <string>
#include <vector>

#ifdef TEST
struct Function
{
    std::set<Function*> dependencies;
    std::vector<Function*> statements;
    std::string toS() { return "fn"; }
};
#endif

// writes the code to the entry point
// c_sources will contain a list of c source files that needs to be included in the gcc call
void generate_code(std::ostream& target, Function* entry_point, std::set<std::string>& c_sources);

