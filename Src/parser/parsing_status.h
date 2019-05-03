#pragma once

#include <iostream>
#include "../utilities/assert.h"

namespace Cube {

enum struct Parsing_status
{
    NOT_PARSED,             // not parsed at all.
    PARTIALLY_PARSED,       // parsed to the point that it's apparent what type of expression it is. Basic info such as token context and owner has been filled in.
    PARSING_IN_PROGRESS,    // is currently being parsed
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

const char* toS(Parsing_status ps);
std::ostream& operator<<(std::ostream& os, Parsing_status ps);
bool is_error(Parsing_status p);
bool is_in_progress(Parsing_status p);
bool is_codegen_ready(Parsing_status p);
bool is_fatal(Parsing_status p);

}
