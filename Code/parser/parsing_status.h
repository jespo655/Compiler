#pragma once


enum struct Parsing_status
{
    NOT_PARSED,             // not parsed at all.
    PARTIALLY_PARSED,       // parsed to the point that it's apparent what type of expression it is.
    FULLY_RESOLVED,         // the expression and all sub-expressions are fully parsed without errors.

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


static is_error(Parsing_status p) {
    if (p == Parsing_status::NOT_PARSED
        || p == Parsing_status::PARTIALLY_PARSED
        || p == Parsing_status::FULLY_RESOLVED
        || p == Parsing_status::DEPENDENCIES_NEEDED)
        return false;
    return true;
}

static is_fatal(Parsing_status p) {
    return p == Parsing_status::FATAL_ERROR;
}