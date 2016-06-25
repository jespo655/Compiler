#pragma once

#include "type.h"
#include "evaluated_variable.h"

#include "../utilities/assert.h"

#include <sstream>
#include <vector>

// literal syntax:
// [int, size=3: 0, 0, 0]
// [int: 0, 0, 0] // size inferred to 3
// [0, 0, 0] // type inferred to int (the type of the first element)

// [int, size=3: 0, 0, 0, 0, 0] // extra values are cut away. This logs a warning.
// [int, size=3: 0] // missing values are default initialized without warning.

// [int, size=3:] // with no values are also ok, but looks a bit wierd
// [int] // a 1 long sequence of types which contains the type int
// [int:] // an empty sequence of ints

// type syntax:
// int[3] // a 3 long sequence of int
// int[..] // a dynamic sequence of int, initially 0 long

struct Type_seq : Type
{
    std::shared_ptr<Type> type;
    int size;
    bool dynamic = false;


    std::string toS(void const * value_ptr, int size=0) const override {
        ASSERT(type != nullptr);
        ASSERT(size == 0 || size == byte_size());
        std::ostringstream oss;
        oss << "[" << type->toS() << ", size=" << size;
        char const * v_ptr = (char const*) value_ptr;
        if (dynamic) {
            oss << ", dynamic";
            ASSERT(sizeof(void*) == 8); // for now, only allow 64 bit
            ASSERT(byte_size() == 16); // FIXME: special case for 32 bit os
            void* const* header = (void* const*)value_ptr;
            ASSERT((uint_least64_t)(header[0]) == size);

            ASSERT(sizeof(char) == 1);
            v_ptr = (char const*)header[1]; // char is used as "byte"
        }
        if (size > 0) {
            oss << ": ";
            int member_size = type->byte_size();
            bool first = true;
            for (int i = 0; i < size; ++i) {
                if (!first) oss << ", ";
                oss << toS(v_ptr + i*member_size, member_size);
                first = false;
            }
        }
        oss << "]";
        return oss.str();
    }


    std::string toS() const override {
        ASSERT(type != nullptr);
        std::ostringstream oss;
        oss << type->toS() << "[";
        if (dynamic) oss << ".."; else oss << size;
        oss << "]";
        return oss.str();
    }

    int byte_size() const override {
        if (dynamic) return sizeof(uint_least64_t) + sizeof(void*); // fat pointer - also stores its own size
        else {
            ASSERT(type != nullptr);
            return size * type->byte_size(); // stores the whole seq on the stack
        }
    }

};



/*
A sequence lookup grabs a data member from a sequence.
It will return exactly one value.
*/
struct Sequence_lookup : Evaluated_variable {

    std::shared_ptr<Evaluated_value> sequence_identifier;
    std::shared_ptr<Evaluated_value> index;

    std::shared_ptr<Type> get_type() override
    {
        ASSERT(sequence_identifier != nullptr);
        auto type = sequence_identifier->get_type();
        if (auto seq_t = std::dynamic_pointer_cast<const Type_seq>(type)) {
            return seq_t->type;
        }
        return nullptr;
    }

    std::string toS() const override { return "sequence lookup"; } // FIXME: better Sequence_lookup::toS()

};
