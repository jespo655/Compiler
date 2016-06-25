#pragma once

#include "type.h"

/*
Strings are just different name for a dynamic sequence of character.
They hold a size and a pointer to string of characters.
*/


struct Type_str : Type
{
    std::string toS(void const * value_ptr, int size=0) const override {
        ASSERT(size == 0 || size == byte_size());
        std::ostringstream oss;

        ASSERT(sizeof(void*) == 8); // for now, only allow 64 bit
        ASSERT(byte_size() == 16); // FIXME: special case for 32 bit os
        void* const* header = (void* const*)value_ptr;
        // copy characters until end of buffer or until \0 is found
        uint_least64_t str_length = (uint_least64_t)header[0];
        char const* char_ptr = (char const*)header[1];
        for (int i = 0; i < str_length; ++i) {
            if (char_ptr[i] == 0) break;
            oss << char_ptr[i];
        }
        return oss.str();
    }

    std::string toS() const override { return "str"; }

    int byte_size() const override { return sizeof(uint_least64_t) + sizeof(void*); } // fat pointer - also stores its own size
};
