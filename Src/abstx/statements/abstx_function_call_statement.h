#pragma once

#include "abstx_statement.h"
#include "../expressions/abstx_function_call.h"

/*
    An function call statement is a line which only calls a function and ignores all return values.
    The function is called only for its side effects.
*/

struct Abstx_function_call_statement : Statement
{
    shared<Abstx_function_call> fc;

    std::string toS() const override { return "function call statement"; }

    Parsing_status finalize() override {
        if (is_codegen_ready(status)) return status;

        if (!is_codegen_ready(fc->status)) {
            if (is_error(fc->status)) status = fc->status;
            else status = Parsing_status::DEPENDENCIES_NEEDED;
            return status;
        }
        // we reached the end -> we are done
        status = Parsing_status::FULLY_RESOLVED;
        return status;
    }

    void generate_code(std::ostream& target) override
    {
        ASSERT(is_codegen_ready(status));
        fc->generate_code(target);
        target << ";" << std::endl;
        status = Parsing_status::CODE_GENERATED;
    }
};
