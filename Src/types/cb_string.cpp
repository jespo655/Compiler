#include "cb_string.h"

using namespace Cube;

void CB_String::generate_type(std::ostream& os) const { os << "_cb_string"; }

void CB_String::generate_typedef(std::ostream& os) const {
    // for now, just use regular null-terminated char*
    os << "typedef char* ";
    generate_type(os);
    os << ";" << std::endl;
}
void CB_String::generate_literal(std::ostream& os, void const* raw_data, uint32_t depth) const {
    ASSERT(raw_data);
    char const* raw_str = *(c_typedef const*)raw_data;
    if (!raw_str) os << "NULL";
    os << "\"" << raw_str << "\"";
}

