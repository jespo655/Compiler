#pragma once

#include "cb_type.h"
#include "../utilities/sequence.h"
#include "../utilities/pointers.h"

#include <string>

/*

T := struct
    {
        a : int = 2;
        b : string;
    };

a : T;
a.a = 4;
a.b = "asd";
a.ásdas; // log_error("ásdas is not a member of a", context);

typeof(T) // type
valueof(T) // unik struct_type (ny för varje "struct"-keyword)


Weight := struct { float kg; }
Length := struct { float m; }

w : Weight;
d : Legnth;

// w = d; // type error

*/




/*

CB_Struct_type har:

* en lista på identifiers (namn+typ), inkl default values / uninitialized, using modifier



CB_Struct har:

* type
* byte array där all data sparas


Datastrukturen följer C standard:
8 bit är inte alignad
16 bit är alignad till 16 bit
32 bit är alignad till 32 bit
64 bit är alignad till 64 bit (endast på 64 bit OS?)

ordning och alignment bestäms dynamiskt av typen och storleken på alla identifiers


// CB:
S :: struct {
    a : i8;
    b : i8;
    c : i16;
};
s : S;
foo(s.a, s.b, s.c);

// Blir C:
uint8_t sa[4];
foo(*((uint8_t*)(sa+0)), *((uint8_t*)(sa+1)), *((uint16_t*)(sa+2)));
// A_INDEX = 0, B_INDEX = 1, C_INDEX = 2

*/

struct CB_Struct : CB_Type
{
    struct Struct_member {
        std::string id;
        shared<CB_Type> type;
        any default_value;
        bool is_using = false; // allowes implicit cast to that member
        bool explicit_uninitialized = false;
        size_t byte_position;

        Struct_member() {};
        Struct_member(const std::string& id, const shared<CB_Type>& type, const any& default_value, bool is_using=false)
            : id{id}, type{type}, default_value{default_value}, is_using{is_using} {};

        std::string toS() const {
            std::ostringstream oss;
            oss << (is_using?"using ":"")
                << id << ":"
                << type->toS() << "="
                << (explicit_uninitialized?"---":default_value.toS());
            return oss.str();
        }
    };

    seq<Struct_member> members;
    void* default_value = nullptr;
    size_t max_alignment = 0;

    // Constructors has to be speficied, otherwise the default move constructor is used when we want to copy
    CB_Struct() {}
    CB_Struct(const CB_Struct& sm) { *this = sm; }
    CB_Struct(CB_Struct&& sm) { *this = std::move(sm); }
    CB_Struct& operator=(const CB_Struct& sm) { uid=sm.uid; members = sm.members; }
    CB_Struct& operator=(CB_Struct&& sm) { uid=sm.uid; members = std::move(sm.members); }
    ~CB_Struct() { free(default_value); }

    std::string toS() const override {
        std::ostringstream oss;
        oss << "struct { ";
        for (int i = 0; i < members.size; ++i) {
            oss << members[i].toS();
            oss << "; ";
        }
        oss << "}";
        return oss.str();
    }

    void add_member(const std::string& id, const shared<CB_Type>& type) {
        members.add(Struct_member(id, type, type->default_value()));
    }

    void finalize() {
        size_t total_size = 0;
        // go through all members, assign them byte positions
        // (TODO: rearrange the members if it would save space)
        for (auto& member : members) {
            // add memory alignment for 16 / 32 bit or bigger values (since this is done in C by default)
            size_t alignment = member.type->alignment();
            total_size += (alignment-total_size%alignment)%alignment;
            member.byte_position = total_size;
            total_size += member.type->cb_sizeof();
            if (alignment > max_alignment) max_alignment = alignment;
        }
        // copy default value
        default_value = malloc(total_size);
        for (auto& member : members) {
            memcpy((uint8_t*)default_value+member.byte_position, member.default_value.v_ptr, member.type->cb_sizeof());
        }
        register_type(toS(), total_size, default_value); // no default value
    }

    operator CB_Type() { return *this; }

    virtual size_t alignment() const override { return max_alignment; }

    // code generation functions
    virtual ostream& generate_typedef(ostream& os) const override {
        os << "typedef struct{ ";
        for (const auto& member : members) {
            // os << std::endl;
            member.type->generate_type(os);
            os << " " << member.id << "; ";
        }
        // if (member.size>0) os << std::endl;
        os << "} ";
        generate_type(os);
        os << ";";
    }
    virtual ostream& generate_literal(ostream& os, void const* raw_data) const override {
        ASSERT(raw_data == nullptr);
        os << "(";
        generate_type(os);
        os << "){";
        for (int i = 0; i << members.size; ++i) {
            if (i) os << ", ";
            members[i].type->generate_literal(os, (uint8_t const*)raw_data+members[i].byte_position);
        }
        return os << "}";
    }
    virtual ostream& generate_destructor(ostream& os, const std::string& id) const override {
        for (const auto& member : members) {
            generate_destructor(os, id + "." + member.id);
        }
        return os;
    };

};




