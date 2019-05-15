#pragma once

#include "cb_type.h"
#include "../utilities/sequence.h"
#include "../utilities/pointers.h"
#include "../abstx/expressions/abstx_identifier.h"

#include <string>

#define ONELINE_STRUCT_DEFINITIONS true

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

// This represents just the type of a struct, not the literal as defined in the code.
struct CB_Struct : CB_Type
{
    struct Struct_member : public Serializable {
        Shared<Abstx_identifier> id; // identifier with possible default value. Owned by declaration statement in the struct literal.
        bool is_using = false; // allowes implicit cast to that member
        bool explicit_uninitialized = false; // not currently used (ignored by the compiler)
        size_t byte_position = 0;

        Struct_member() {}
        Struct_member(const Shared<Abstx_identifier>& id, bool is_using=false) : id{id}, is_using{is_using} {}
        ~Struct_member() {}

        std::string toS() const override;
    };

    static void* const _default_empty_value;

    Seq<Struct_member> members;
    void* _default_value = nullptr;
    size_t max_alignment = 0;

    // Constructors has to be speficied, otherwise the default move constructor is used when we want to copy
    CB_Struct() { uid = get_unique_type_id(); } // we can't finalize here because finalize can only be done once. Just set uid so it can be referenced later
    CB_Struct(const CB_Struct& sm) { *this = sm; }
    CB_Struct(CB_Struct&& sm) { *this = std::move(sm); }
    CB_Struct& operator=(const CB_Struct& sm);
    CB_Struct& operator=(CB_Struct&& sm);
    ~CB_Struct();

    // Convenience constructors - assumes that no members are using
    CB_Struct(const Seq<Shared<Abstx_identifier>>& members) { for (const auto& member : members) { add_member(member); } finalize(); }
    CB_Struct(const Seq<Owned<Abstx_identifier>>& members) { for (const auto& member : members) { add_member(Shared<Abstx_identifier>(member)); } finalize(); }
    CB_Struct(int) { finalize(); } // explicit empty

    std::string toS() const override;

    bool is_primitive() const override { return false; }

    void add_member(const Shared<Abstx_identifier>& id, bool is_using=false);
    Shared<const Struct_member> get_member(const std::string& id) const;

    operator CB_Type() { return *this; }

    virtual size_t alignment() const override { return max_alignment; }

    void finalize() override;

    // code generation functions
    void generate_typedef(std::ostream& os) const override;
    void generate_literal(std::ostream& os, void const* raw_data, const Token_context& context, uint32_t depth = 0) const override;
    void generate_destructor(std::ostream& os, const std::string& id, const Token_context& context, uint32_t depth = 0) const override;

private:
    static void align(size_t* v, size_t alignment) {
        if (alignment == 0) alignment = 1;
        *v += (alignment - *v % alignment) % alignment;
    }
};
