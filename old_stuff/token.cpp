#include "token.h"
#include <iostream>

std::ostream& operator << (std::ostream& os, const Token_context& context)
{
    if (!context.file.empty()) {
        os << "(In " << context.file << ", ";
    } else {
        os << "(At ";
    }
    os << "line " << context.line << ", position " << context.position << ")";
    return os;
}


std::ostream& operator << (std::ostream& os, const Token_type& type)
{
    switch (type) {
        case Token_type::UNKNOWN: return os << "UNKNOWN";
        case Token_type::SYMBOL: return os << "SYMBOL";
        case Token_type::IDENTIFIER: return os << "IDENTIFIER";
        case Token_type::KEYWORD: return os << "KEYWORD";
        case Token_type::INTEGER: return os << "INTEGER";
        case Token_type::FLOAT: return os << "FLOAT";
        case Token_type::STRING: return os << "STRING";
        case Token_type::BOOL: return os << "BOOL";
        case Token_type::EOF: return os << "EOF";
        default: return os << "???";
    }
}
