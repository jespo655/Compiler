#pragma once

#include "abstx_statement.h"

/*
Raw C code is necessary in order to call C-native functions.



syntax:
    #C string_literal
example:
    #C_CODE #string C_CODE printf("hello world!"); C_CODE;
generates:
    printf("hello world!");


syntax: (extension; not yet implemented)
    #C compile_time_known_string
example:
    c_code :: "printf("hello world!");";
    #C_CODE c_code;
generates:
    printf("hello world!");
*/

struct Abstx_c_code : Statement {

    std::string c_code = "";

    std::string toS() const override {
        return c_code;
    }

    Parsing_status finalize() override {
        if (is_codegen_ready(status)) return status;
        // we reached the end -> we are done
        status = Parsing_status::FULLY_RESOLVED;
        return status;
    }

    void generate_code(std::ostream& target) override {
        ASSERT(is_codegen_ready(status));
        target << c_code << std::endl;
        status = Parsing_status::CODE_GENERATED;
    };

};
