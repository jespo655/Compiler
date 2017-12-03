#pragma once

#include "../abstx/scope.h"
#include "compile_time.h"


/*
A workspace is a place where code can be executed
One workspace is created for each #run command
One workspace is reserved for the compiled output

When the code is run, the owner of the global scope of the entry point is set as the AST_root of the workspace
The workspace is also set as the owner of the global scope. That way the scope can get access to compile time functions.
*/
struct Workspace : Scope {

    bool compile_time = false; // is true if the workspace is created from a #run method
    std::shared_ptr<Scope> AST_root;

    // Function + arguments creates a function call that defines the entry point
    std::shared_ptr<Function> entry_point;
    std::vector<std::shared_ptr<Value_expression>> arguments;

    // c include paths are inserted at the top of the file (not for compile time)
    std::vector<std::string> c_includes;


    std::shared_ptr<Function> get_function(const std::string& id, bool recursive=true) override
    {
        // FIXME: if compile time, get compile time functions
        // if (compile_time) return get_compile_time_function(id);
        // else
        return nullptr;
    }


    // global scope is probably not needed, so it's commented out for now
    // std::shared_ptr<Scope> global_scope() const override
    // {
    //     return AST_root;
    // }

};

