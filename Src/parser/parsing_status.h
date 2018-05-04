#pragma once

#include <iostream>

enum struct Parsing_status
{
    NOT_PARSED,             // not parsed at all.
    PARTIALLY_PARSED,       // parsed to the point that it's apparent what type of expression it is. Basic info such as token context and owner has been filled in.
    FULLY_PARSED,           // the expression and all sub-expressions are fully parsed without errors, but some dependencies might not be done yet. Set with abstx->fully_parse()
    FULLY_RESOLVED,         // the expression and all sub-expressions are fully parsed without errors. Set with abstx->finalize()
    CODE_GENERATED,         // the expression has been fully parsed and code has been outputted to file. Set with abstx->generate_code()

    DEPENDENCIES_NEEDED,    // the expression cannot yet be fully parsed because it's waiting for other things to parse first

    // Error codes: the expression cannot be fully parsed because:
    SYNTAX_ERROR,           // there was a syntax error
    TYPE_ERROR,             // some types doesn't match
    CYCLIC_DEPENDENCY,      // there was a cyclic dependency
    COMPILE_TIME_ERROR,     // there was an error in a #run (that was not one of the above error types)
    UNDECLARED_IDENTIFIER,  // it tried to use an identifier which was not yet declared
    FATAL_ERROR,            // there was an error so bad that we couldn't recover. Continued compilation would give undefined behaviour.

    // TODO: Add more error types as they pop up

};

static std::ostream& operator<<(std::ostream& os, Parsing_status ps) {
    switch(ps) {
        case Parsing_status::NOT_PARSED: return os << "NOT_PARSED";
        case Parsing_status::PARTIALLY_PARSED: return os << "PARTIALLY_PARSED";
        case Parsing_status::FULLY_PARSED: return os << "FULLY_PARSED";
        case Parsing_status::FULLY_RESOLVED: return os << "FULLY_RESOLVED";
        case Parsing_status::CODE_GENERATED: return os << "CODE_GENERATED";
        case Parsing_status::DEPENDENCIES_NEEDED: return os << "DEPENDENCIES_NEEDED";
        case Parsing_status::SYNTAX_ERROR: return os << "SYNTAX_ERROR";
        case Parsing_status::TYPE_ERROR: return os << "TYPE_ERROR";
        case Parsing_status::CYCLIC_DEPENDENCY: return os << "CYCLIC_DEPENDENCY";
        case Parsing_status::COMPILE_TIME_ERROR: return os << "COMPILE_TIME_ERROR";
        case Parsing_status::UNDECLARED_IDENTIFIER: return os << "UNDECLARED_IDENTIFIER";
        case Parsing_status::FATAL_ERROR: return os << "FATAL_ERROR";
    }
    return os;
}

static is_error(Parsing_status p) {
    if (p == Parsing_status::NOT_PARSED
        || p == Parsing_status::PARTIALLY_PARSED
        || p == Parsing_status::FULLY_RESOLVED
        || p == Parsing_status::CODE_GENERATED
        || p == Parsing_status::DEPENDENCIES_NEEDED)
        return false;
    return true;
}

static is_codegen_ready(Parsing_status p) {
    return p == Parsing_status::FULLY_RESOLVED || p == Parsing_status::CODE_GENERATED;
}

static is_fatal(Parsing_status p) {
    return p == Parsing_status::FATAL_ERROR;
}
