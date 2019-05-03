#include "parsing_status.h"
#include "../utilities/assert.h"

const char* toS(Parsing_status ps) {
    switch(ps) {
        case Parsing_status::NOT_PARSED: return "NOT_PARSED";
        case Parsing_status::PARTIALLY_PARSED: return "PARTIALLY_PARSED";
        case Parsing_status::PARSING_IN_PROGRESS: return "PARSING_IN_PROGRESS";
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

std::ostream& operator<<(std::ostream& os, Parsing_status ps) {
    return os << toS(ps);
}

bool is_error(Parsing_status p) {
    if (p == Parsing_status::NOT_PARSED
        || p == Parsing_status::PARTIALLY_PARSED
        || p == Parsing_status::PARSING_IN_PROGRESS
        || p == Parsing_status::FULLY_RESOLVED
        || p == Parsing_status::DEPENDENCIES_NEEDED)
        return false;
    return true;
}

bool is_in_progress(Parsing_status p) {
    return p == Parsing_status::PARSING_IN_PROGRESS;
}

bool is_codegen_ready(Parsing_status p) {
    return p == Parsing_status::FULLY_RESOLVED;
}

bool is_fatal(Parsing_status p) {
    return p == Parsing_status::FATAL_ERROR;
}
