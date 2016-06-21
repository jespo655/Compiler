#pragma once

#include "scope.h"
#include "../compile_time/compile_time.h"


struct Workspace : Scope {

    bool compile_time = false; // is true if the workspace is created from a #run method
    std::shared_ptr<Scope> AST_root;

    std::shared_ptr<Function> entry_point;

    std::vector<std::shared_ptr<Evaluated_value>> arguments;


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

