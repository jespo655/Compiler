#pragma once

#include "cb_type.h"
#include "../utilities/sequence.h"
#include "../utilities/pointers.h"
#include "../abstx/expressions/abstx_identifier.h"

#include <string>
#include <iomanip>

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
        Abstx_identifier id;
        bool is_using = false; // allowes implicit cast to that member
        bool explicit_uninitialized = false; // not currently used (ignored by the compiler)
        size_t byte_position;

        Struct_member() {};
        Struct_member(const std::string& name, const Any& value, bool is_using=false) : id{}, is_using{is_using} {
            id.name = name;
            id.value = value;
        };

        std::string toS() const {
            std::ostringstream oss;
            oss << (is_using?"using ":"")
                << id.name << ":"
                << type->toS() << "="
                << (explicit_uninitialized?"---":id.value.toS());
            return oss.str();
        }
    };

    Seq<Struct_member> members;
    void* _default_value = nullptr;
    size_t max_alignment = 0;

    // Constructors has to be speficied, otherwise the default move constructor is used when we want to copy
    CB_Struct() {}
    CB_Struct(const CB_Struct& sm) { *this = sm; }
    CB_Struct(CB_Struct&& sm) { *this = std::move(sm); }
    CB_Struct& operator=(const CB_Struct& sm) {
        uid=sm.uid; members = sm.members; max_alignment=sm.max_alignment;
        _default_value = malloc(sm.cb_sizeof());
        memcpy(_default_value, sm._default_value, sm.cb_sizeof());
    }
    CB_Struct& operator=(CB_Struct&& sm) {
        uid=sm.uid;
        members = std::move(sm.members);
        _default_value = sm._default_value; sm._default_value = nullptr;
        max_alignment=sm.max_alignment;
    }
    ~CB_Struct() { free(_default_value); }

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

    bool is_primitive() const override { return false; }

    void add_member(const std::string& id, const Shared<const CB_Type>& type) {
        ASSERT(type != nullptr);
        members.add(Struct_member(id, type->default_value()));
    }

    void finalize() {
        size_t total_size = 0;
        if (members.empty()) {
            total_size = 1;
            _default_value = malloc(total_size);
            // actual default value doesn't matter since it will never be used anyway
        } else {
            // go through all members, assign them byte positions
            // (TODO: rearrange the members if it would save space)
            for (auto& member : members) {
                // add memory alignment for 16 / 32 bit or bigger values (since this is done in C by default)
                size_t alignment = member.id.value.v_type->alignment();
                align(&total_size, alignment);
                member.byte_position = total_size;
                total_size += member.id.value.v_type->cb_sizeof();
                if (alignment > max_alignment) max_alignment = alignment;
            }

            // fix final alignment so it works in sequences
            align(&total_size, max_alignment);
            // copy default value
            _default_value = malloc(total_size);
            for (auto& member : members) {
                memcpy((uint8_t*)_default_value+member.byte_position, member.id.value.v_ptr, member.id.value.v_type->cb_sizeof());
            }
        }
        ASSERT(total_size > 0);
        ASSERT(_default_value != nullptr);
        register_type(toS(), total_size, _default_value);
    }

    operator CB_Type() { return *this; }

    virtual size_t alignment() const override { return max_alignment; }

    // code generation functions
    void generate_typedef(ostream& os) const override {
        ASSERT(_default_value); // assert finalized
        os << "typedef struct{ ";
        for (const auto& member : members) {
            // os << std::endl;
            member.id.value.v_type->generate_type(os);
            os << " " << member.id.name << "; ";
        }
        // if (member.size>0) os << std::endl;
        os << "} ";
        generate_type(os);
        os << ";" << std::endl;
    }
    void generate_literal(ostream& os, void const* raw_data, uint32_t depth = 0) const override {
        ASSERT(_default_value, "struct must be finalized before it can be used"); // assert finalized
        ASSERT(raw_data != nullptr);
        if (depth > MAX_ALLOWED_DEPTH) { post_circular_reference_error(); os << "void"; return; }
        os << "(";
        generate_type(os);
        os << "){";
        for (int i = 0; i < members.size; ++i) {
            if (i) os << ", ";
            members[i].id.value.v_type->generate_literal(os, (uint8_t const*)raw_data+members[i].byte_position);
        }
        os << "}";
    }
    void generate_destructor(ostream& os, const std::string& id, uint32_t depth = 0) const override {
        if (depth > MAX_ALLOWED_DEPTH) { post_circular_reference_error(); return; }
        for (const auto& member : members) {
            member.id.value.v_type->generate_destructor(os, id + "." + member.id.name, depth+1);
        }
    };

private:
    static void align(size_t* v, size_t alignment) {
        *v += (alignment - *v % alignment) % alignment;
    }
};




