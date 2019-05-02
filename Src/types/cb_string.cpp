#include "cb_string.h"

namespace Cube {

void CB_String::generate_type(ostream& os) const override { os << "_cb_string"; }

void CB_String::generate_typedef(ostream& os) const override {
    // for now, just use regular null-terminated char*
    os << "typedef char* ";
    generate_type(os);
    os << ";" << std::endl;
}
void CB_String::generate_literal(ostream& os, void const* raw_data, uint32_t depth = 0) const override {
    ASSERT(raw_data);
    char const* raw_str = *(c_typedef const*)raw_data;
    if (!raw_str) os << "NULL";
    os << "\"" << raw_str << "\"";
}

}
