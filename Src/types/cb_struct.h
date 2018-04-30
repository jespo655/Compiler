#pragma once

#include "cb_type.h"
// #include "pointers.h"
// #include "any.h"
// #include "string.h"
// #include "primitives.h"

#include "sequence.h"

#include <string>

#error @TODO: not done yet

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

typedef uint8_t byte;

struct CB_Struct : CB_Type
{
    struct Struct_member {
        CB_String id;
        CB_Type type;
        CB_Any default_value;
        CB_Bool is_using = false; // allowes implicit cast to that member
        CB_Bool explicit_uninitialized = false;

        Struct_member() {};
        Struct_member(const CB_String& id, const CB_Type& type, const CB_Any& default_value, bool is_using=false)
            : id{id}, type{type}, default_value{default_value}, is_using{is_using} {};

        std::string toS() const { return std::string("Struct_member(") + (is_using?"using ":"") + id.toS() + ":" + type.toS() + "=" + (explicit_uninitialized?"---":default_value.toS()) + ")"; }
    };

    seq<Struct_member> members;


    // Constructors has to be speficied, otherwise the default move constructor is used when we want to copy
    CB_Struct_type() : CB_Type{} {}
    CB_Struct_type(const CB_Struct_type& sm) { *this = sm; }
    CB_Struct_type(CB_Struct_type&& sm) { *this = std::move(sm); }
    CB_Struct_type& operator=(const CB_Struct_type& sm) { uid=sm.uid; members = sm.members; }
    CB_Struct_type& operator=(CB_Struct_type&& sm) { uid=sm.uid; members = std::move(sm.members); }
    ~CB_Struct_type() {}

    std::string toS() const override {
        std::ostringstream oss;
        oss << "struct { ";
        for (int i = 0; i < members.size(); ++i) {
            if (i > 0) oss << "; ";
            oss << members[i].toS();
        }
        oss << " }";
        return oss.str();
    }
    CB_Object* heap_copy() const override { CB_Struct_type* tp = new CB_Struct_type(); *tp = *this; return tp; }

    bool operator==(const CB_Struct_type& o) const { return (CB_Type)*this == (CB_Type)o; } // different struct types are all different
    bool operator!=(const CB_Struct_type& o) const { return !(*this==o); }
    bool operator==(const CB_Type& o) const { toS() == o.toS(); }
    bool operator!=(const CB_Type& o) const { return !(*this==o); }
    operator CB_Type() { return *this; }

    // CB_Struct_pointer operator()(); // "constructor operator", creates an instance of this struct_type

    size_t byte_size();
    void set_default_values(byte* data);
    size_t get_member_index(const std::string& member); // returns the index of the specified member, or -1 if the member doesn't exist
    CB_Sharing_pointer<CB_Type> get_member_type(const std::string& member); // returns the index of the specified member, or -1 if the member doesn't exist

};




