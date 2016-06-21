#pragma once

#include "type.h"
#include "literal.h"
#include "evaluated_variable.h"

#include "../utilities/assert.h"

#include <sstream>
#include <vector>


// [int, size=3: 0, 0, 0]

struct Literal_seq : Literal
{
    std::shared_ptr<const Type> type;
    int size;
    std::vector<std::shared_ptr<Evaluated_value>> members;

    std::string toS() const override {
        ASSERT(type != nullptr);
        std::ostringstream oss;
        oss << "[" << type->toS() << ", size=" << size;
        ASSERT(members.size() == size);
        if (!members.empty()) oss << ": ";
        bool first = true;
        for (auto m : members) {
            ASSERT(m != nullptr);
            ASSERT(m->get_type() == type);
            if (!first) oss << ", ";
            oss << m->toS();
            first = false;
        }
        oss << "]";
        return oss.str();
    }

    std::shared_ptr<const Type> get_type() override;
};


// int[3]
// int[..]

struct Type_seq : Type
{
    std::shared_ptr<const Type> type;
    int size;
    bool dynamic = false;

    std::string toS() const override {
        ASSERT(type != nullptr);
        std::ostringstream oss;
        oss << type->toS() << "[";
        if (dynamic) oss << ".."; else oss << size;
        oss << "]";
        return oss.str();
    }

    std::shared_ptr<const Literal> get_default_value() const override
    {
        default_literal->type = type;
        default_literal->size = size;
        default_literal->members.resize(size);
        return default_literal;
    }

private:
    std::shared_ptr<Literal_seq> default_literal{new Literal_seq()};
};



/*
A sequence lookup grabs a data member from a sequence.
It will return exactly one value.
*/
struct Sequence_lookup : Evaluated_variable {

    std::shared_ptr<Evaluated_value> sequence_identifier;
    std::shared_ptr<Evaluated_value> index;

    std::shared_ptr<const Type> get_type() override
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
