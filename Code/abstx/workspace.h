#pragma once

#include "scope.h"
#include "../compile_time/compile_time.h"


/*
A workspace is a place where code can be executed
One workspace is created for each #run command
One workspace is reserved for the compiled output
*/
struct Workspace : Scope {

    bool compile_time = false; // is true if the workspace is created from a #run method
    std::shared_ptr<Scope> AST_root;

    // Function + arguments creates a function call that defines the entry point
    std::shared_ptr<Function> entry_point;
    std::vector<std::shared_ptr<Value_expression>> arguments;

    // c include paths are inserted at the top of the file
    std::vector<std::string> c_includes;


    std::shared_ptr<Function> get_function(const std::string& id, bool recursive=true) override
    {
        // if (compile_time) return get_compile_time_function(id);
        // else
        return nullptr;
    }


    std::shared_ptr<Scope> global_scope() const override
    {
        return AST_root;
    }





};

