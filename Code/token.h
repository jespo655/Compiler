#pragma once

#include "utilities/assert.h"

#include <string>
#include <sstream>

struct Token_context
{
    int line = 1;
    int position = 0;
    std::string file{};

    bool operator==(const Token_context& tc) const { return line==tc.line && position==tc.position && file==tc.file; }
    bool operator!=(const Token_context& tc) const { return !(*this==tc); }
    bool operator<(const Token_context& tc) const
    {
        ASSERT(file==tc.file, "TODO: decide what happens if comparing contexts from different files.");
        if (line < tc.line) return true;
        if (line == tc.line && position < tc.position) return true;
        return false;
    }
    bool operator<=(const Token_context& tc) const
    {
        ASSERT(file==tc.file, "TODO: decide what happens if comparing contexts from different files.");
        if (line < tc.line) return true;
        if (line == tc.line && position <= tc.position) return true;
        return false;
    }
    bool operator>(const Token_context& tc) const { return !(*this<=tc); }
    bool operator>=(const Token_context& tc) const { return !(*this<tc); }

    std::string toS() const {
        std::ostringstream oss;
        if (!file.empty()) {
            oss << "(In " << file << ", ";
        } else {
            oss << "(At ";
        }
        oss << "line " << line << ", position " << position << ")";
        return oss.str();
    }

};

