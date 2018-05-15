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

    Parsing_status fully_parse() override; // implemented in statement_parser.cpp

    void generate_code(std::ostream& target) const override {
        ASSERT(is_codegen_ready(status));
        target << c_code << std::endl;
    };

};
