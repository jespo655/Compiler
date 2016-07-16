#pragma once

#include "type.h"

/*
Strings are just different name for a dynamic sequence of character.
They hold a size and a pointer to string of characters.
*/


// fat pointer - also stores its own size
struct Str_header {
    uint_least64_t len; // not needed if we don't allow [] operator for strings (each string is an atom)
    char* v; // '\0' terminated array of characters. byte size = len+1, counting the mandatory '\0'
};


struct Type_str : Type
{
    static std::string cpp_value(void const* value_ptr, int size=0) {
        ASSERT(size == 0 || size == byte_size());
        std::ostringstream oss;

        Str_header* const header_ptr = (Str_header* const)value_ptr;
        // copy characters until end of buffer or until '\0' is found
        // FIXME: optimization, strcpy?
        // FIXME: maybe skip the size in the header? The string is always '\0' terminated anyways (or should be, at least)

        oss << header_ptr->v; // should be '\0' terminated so everything is fine
        // for (int i = 0; i < header_ptr->len; ++i) {
        //     if (header_ptr->v[i] == '\0') break;
        //     oss << char_ptr[i];
        // }
        return oss.str();
    }

    std::string toS(void const * value_ptr, int size=0) const override {
        return cpp_value(value_ptr, size);
    }

    std::string toS() const override { return "str"; }

    int byte_size() const override { return sizeof(Str_header); }
};
