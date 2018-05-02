#include "code_generator.h"
#include <algorithm>
#include <iostream>


void recursive_code_generation(std::ostream& target, Function* abstx_node) {
    for (auto& dependency : abstx_node->dependencies) {
        recursive_code_generation(target, dependency);
    }

    // TODO: generate real code for each statement
    target << std::endl << "// Code for " << abstx_node->toS();
}





void generate_code(std::ostream& target, Function const* entry_point, const std::set<std::string>& c_sources)
{
    // 1) find all c include and source files
    std::set<std::string> c_includes;

    // TODO: through the entry point, populate c_includes and c_sources with all dependencies

    target << std::endl << "/* Code generated with Cube compiler */";
    for (auto& inc : c_includes) {
        target << std::endl << "#include " << inc << ";";
    }
    target << std::endl;

    // generate code for all dependencies
    for (auto& dependency : entry_point->dependencies) {
        recursive_code_generation(target, dependency);
    }

    // generate code for main function
    target << std::endl << "int main(int argc, char* argv[]) {" << std::endl;
    for (auto& statement : entry_point->statements) {

        // TODO: generate real code for each statement
        target << "// " << statement->toS() << std::endl;
    }
    target << "}" << std::endl;

    target << std::endl << "/* End of code */" << std::endl;
}

// void generate_assignment_code(std::ostream& target, Shared<Assignment_statement> as);






#ifdef TEST

int main()
{
    Function fn;
    std::set<std::string> c_sources;
    generate_code(std::cout, &fn, c_sources);
}

#endif

