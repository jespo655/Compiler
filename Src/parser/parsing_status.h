#pragma once

#include <iostream>

enum struct Parsing_status
{
    NOT_PARSED,             // not parsed at all.
    PARTIALLY_PARSED,       // parsed to the point that it's apparent what type of expression it is. Basic info such as token context and owner has been filled in.
    FULLY_RESOLVED,         // the expression and all sub-expressions are fully parsed without errors.

    DEPENDENCIES_NEEDED,    // the expression cannot yet be fully parsed because it's waiting for other things to parse first

    // Error codes: the expression cannot be fully parsed because:
    SYNTAX_ERROR,           // there was a syntax error
    TYPE_ERROR,             // some types doesn't match
    CYCLIC_DEPENDENCY,      // there was a cyclic dependency
    COMPILE_TIME_ERROR,     // there was an error in a #run (that was not one of the above error types)
    UNDECLARED_IDENTIFIER,  // it tried to use an identifier which was not yet declared
    REDECLARED_IDENTIFIER,  // it tried to declare an identifier which was already declared
    FATAL_ERROR,            // there was an error so bad that we couldn't recover. Continued compilation would give undefined behaviour.

    // TODO: Add more error types as they pop up

};

static std::string toS(Parsing_status ps) {
    switch(ps) {
        case Parsing_status::NOT_PARSED: return "NOT_PARSED";
        case Parsing_status::PARTIALLY_PARSED: return "PARTIALLY_PARSED";
        case Parsing_status::FULLY_RESOLVED: return "FULLY_RESOLVED";
        case Parsing_status::DEPENDENCIES_NEEDED: return "DEPENDENCIES_NEEDED";
        case Parsing_status::SYNTAX_ERROR: return "SYNTAX_ERROR";
        case Parsing_status::TYPE_ERROR: return "TYPE_ERROR";
        case Parsing_status::CYCLIC_DEPENDENCY: return "CYCLIC_DEPENDENCY";
        case Parsing_status::COMPILE_TIME_ERROR: return "COMPILE_TIME_ERROR";
        case Parsing_status::UNDECLARED_IDENTIFIER: return "UNDECLARED_IDENTIFIER";
        case Parsing_status::REDECLARED_IDENTIFIER: return "REDECLARED_IDENTIFIER";
        case Parsing_status::FATAL_ERROR: return "FATAL_ERROR";
    }
    ASSERT(false);
    return "UNKNOWN_STATUS";
}

static std::ostream& operator<<(std::ostream& os, Parsing_status ps) {
    return os << toS(ps);
}

static bool is_error(Parsing_status p) {
    if (p == Parsing_status::NOT_PARSED
        || p == Parsing_status::PARTIALLY_PARSED
        || p == Parsing_status::FULLY_RESOLVED
        || p == Parsing_status::DEPENDENCIES_NEEDED)
        return false;
    return true;
}

static bool is_codegen_ready(Parsing_status p) {
    return p == Parsing_status::FULLY_RESOLVED;
}

static bool is_fatal(Parsing_status p) {
    return p == Parsing_status::FATAL_ERROR;
}
