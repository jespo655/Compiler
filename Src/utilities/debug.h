#pragma once

#include <iostream>
#include <sstream>

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


// Abstract class to be extended by all classes that could be outputted as strings
struct Serializable
{
    // debug_print(): print all data about the thing, on one or several lines.
    // If recursive=true, then also call debug_print() on all child nodes.
    // For child nodes, use os.indent() and os.unindent() for clarity.
    // As default debug_print() prints toS().
    virtual void debug_print(Debug_os& os, bool recursive=true) const { os << toS() << std::endl; }

    // toS(): concatenate the most basic data about the node in a single
    // line of text. This line should preferrably be self contained and fit
    // well into other text.
    // This method should always be implemented in all non-abstract classes.
    virtual std::string toS() const = 0;
};

inline std::ostream& operator<<(std::ostream& os, const Serializable& s) { return os << s.toS(); }

template<typename... Args>
std::string toS(void (*fn)(std::ostream& os, Args... args), Args... args) {
    std::stringstream ss{};
    fn(ss, args...);
    return ss.str();
}

template<typename T, typename Super, typename... Args>
std::string toS(const T& t, void (Super::*fn)(std::ostream& os, Args... args) const, Args... args) {
    std::stringstream ss{};
    (t.*fn)(ss, args...);
    return ss.str();
}

template<typename... Args>
std::string toS(void (*fn)(Debug_os& os, Args... args), Args... args) {
    std::stringstream ss{};
    Debug_os dos{ss};
    fn(dos, args...);
    return ss.str();
}

template<typename T, typename Super, typename... Args>
std::string toS(const T& t, void (Super::*fn)(Debug_os& os, Args... args) const, Args... args) {
    std::stringstream ss{};
    Debug_os dos{ss};
    (t.*fn)(dos, args...);
    return ss.str();
}

#ifdef DEBUG
#define LOG(s) \
    do { \
        std::cout << __FILE__ << ":" << __LINE__ << ": " << s << std::endl; \
    } while(0)
#else
#define LOG(s) void(0)
#endif

