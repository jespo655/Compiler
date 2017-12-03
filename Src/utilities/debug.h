#pragma once

#include <iostream>

#define DEBUG // undef this flag to remove all debug output


/*
    Debug_os works as a regular ostream, but with a few quirks.

    using indent(), unindent() and set_indentation(), all printed lines
    will be prefixed by N spaces per indentation level.

    The number N can be changed by changing spaces_per_indent.
    The default is 4.
*/

struct Debug_os {

    Debug_os(std::ostream& os) : os{os} {}

    int spaces_per_indent = 4;
    int indent_level = 0;
    bool should_indent = true;

    template<typename T> inline Debug_os& operator<<(const T& t)
    {
        #ifdef DEBUG
            output_indentation();
            os << t;
        #endif
        return *this;
    }

    inline Debug_os& operator<<(std::ostream&(*f)(std::ostream&))
    {
        #ifdef DEBUG
            output_indentation();
            os << f;
            if(f == (std::ostream&(*)(std::ostream&))std::endl) {
                should_indent = true;
            }
        #endif
        return *this;
    }

    void indent(int steps = 1) { indent_level += steps; }
    void unindent(int steps = 1) { indent_level -= steps; }
    void set_indent_level(int level) { indent_level = level; }

private:
    std::ostream& os;

    void output_indentation() {
        if (should_indent) {
            for (int i = 0; i < indent_level*spaces_per_indent; ++i) os << ' ';
            should_indent = false;
        }
    }
};