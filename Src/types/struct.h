#pragma once

#include "type.h"
#include "pointers.h"
#include "any.h"
#include "string.h"
#include "primitives.h"
#include "dynamic_seq.h"
#include <string>
#include <vector>

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

struct CB_Struct_type : CB_Type
{
    static CB_Type type; // self reference / CB_Type

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

    // CB_Dynamic_seq<Struct_member> members;
    std::vector<Struct_member> members;


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





// holds a pointer to the actual struct
// can be used to access the struct members
struct CB_Struct_pointer {
    static CB_Type type;
    CB_Sharing_pointer<CB_Struct_type> struct_type;

    bool owning;
    byte* data;

    CB_Struct_pointer() {}
    ~CB_Struct_pointer() { if (owning) free(data); }
    CB_Struct_pointer(void* data) : data{(byte*)data}, owning{false} {}
    CB_Struct_pointer(const CB_Sharing_pointer<CB_Struct_type>& s_type, bool explicit_uninitialized = false) : owning{false}, struct_type{s_type}
    {
        data = (byte*)malloc(struct_type->byte_size());
        if (!explicit_uninitialized) struct_type->set_default_values(data);
    }

    // returns null if member doesn't exist
    byte* get_member_pointer(const std::string& member) {
        size_t index = struct_type->get_member_index(member);
        if (index == (size_t)-1) return nullptr;
        return data+index;
    }



};










/*
    Struct_instance(const Struct_type& struct_type) : struct_type{struct_type} { init_members(); };

    // copy and move constructors, destructor
    Struct_instance(const Struct_instance& si) : struct_type{si.struct_type} { *this = si; }
    Struct_instance(Struct_instance&& si) : struct_type{std::move(si.struct_type)} { *this = std::move(si); }
    Struct_instance& operator=(const Struct_instance& si) { ASSERT(struct_type == si.struct_type); values = si.values; }
    Struct_instance& operator=(Struct_instance&& si) { ASSERT(struct_type == si.struct_type); values = std::move(si.values); }
    ~Struct_instance() {}

    int get_CB_size() { ASSERT(false, "NYI"); } // TODO: implement

    bool is_same_type(const CB_Any& any, const CB_Type& type) const {
        // compare the type of an any with a CB_Type
        // if the any is of a struct type, compare its value's struct_type
        // else, compare its v_type

        if (any.v_type == Struct_instance::type) {
            // the value is a struct, so we should compare struct types
            return (CB_Type)any.value<Struct_instance>().struct_type == type;
        }
        else return any.v_type == type;
    }

    std::string toS() const {
        ASSERT(struct_type.metadata != nullptr);
        ASSERT(values.size == struct_type.metadata->members.size);
        std::ostringstream oss;

        const std::string& type_name = struct_type.typenames[struct_type.uid];
        if (type_name == "") oss << "instance of anonymous struct_type("+std::to_string(struct_type.uid)+")";
        else oss << type_name;

        oss << "(";
        for (int i = 0; i < values.size; ++i) {
            if (i > 0) oss << " ";
            Struct_member& member_metadata = struct_type.metadata->members[i];
            oss << member_metadata.id.toS() << ":";
            oss << member_metadata.type.toS() << "=";
            if (member_metadata.explicit_uninitialized) oss << "---";
            else {
                // get the current value
                ASSERT(is_same_type(values[i], member_metadata.type));
                oss << values[i].toS();
            }

            oss << ";";
        }
        oss << ")";
        return oss.str();
    }

    void init_members() {
        ASSERT(struct_type.metadata != nullptr);
        values.clear();
        for (auto& member : struct_type.metadata->members) {
            values.add(member.default_value);
        }
    }

    CB_Any& member(const CB_String& name) {
        ASSERT(struct_type.metadata != nullptr);
        ASSERT(values.size == struct_type.metadata->members.size);

        CB_Sharing_pointer<Struct_instance> used_struct_with_member = nullptr;

        for (int i = 0; i <  values.size; ++i) {
            Struct_member& member_metadata = struct_type.metadata->members[i];
            if (member_metadata.id == name) {
                ASSERT(is_same_type(values[i], member_metadata.type));
                return values[i];
            }

            // If a struct member is marked with the 'using' keyword, its members can also be reached.
            if (member_metadata.is_using && values[i].v_type == Struct_instance::type) {
                if (values[i].value<Struct_instance>().has_member(name)) {
                    ASSERT(used_struct_with_member == nullptr, "Ambiguous struct member: "+name.toS());
                    used_struct_with_member = &values[i].value<Struct_instance>();
                }
            }
        }

        if (used_struct_with_member != nullptr) return used_struct_with_member->member(name);

        ASSERT(false, "Cannot find member "+name.toS()+" in "+toS()); // log_error?
    }

    bool has_member(const CB_String& name) const {
        ASSERT(struct_type.metadata != nullptr);
        ASSERT(values.size == struct_type.metadata->members.size);
        for (int i = 0; i <  values.size; ++i) {
            const Struct_member& member_metadata = struct_type.metadata->members[i];
            if (member_metadata.id == name) return true;

            // If a struct member is marked with the 'using' keyword, its members can also be reached.
            if (member_metadata.is_using && values[i].v_type == Struct_instance::type) {
                auto member = values[i].value<Struct_instance>();
                if (member.has_member(name)) return true;
            }
        }
        return false;
    }

*/













/*
    FIXME: at assignment, assert that the struct_types are the same
    Also make special case for this when using higher order structures such as CB_Any
*/

/*
struct Struct_member {
    CB_String id;
    CB_Type type;
    CB_Any default_value;
    CB_Bool is_using = false; // allowes implicit cast to that member
    CB_Bool explicit_uninitialized = false;

    Struct_member() {};
    Struct_member(const CB_String& id, const CB_Type& type, const CB_Any& default_value, bool is_using=false)
        : id{id}, type{type}, default_value{default_value}, is_using{is_using} {};

    std::string toS() const { return "Struct_member(" + id.toS() + ":" + type.toS() + "=" + default_value.toS() + ")"; }
};

struct Struct_metadata {
    CB_Dynamic_seq<Struct_member> members;

    // Constructors has to be speficied, otherwise the default move constructor is used when we want to copy
    Struct_metadata() {};
    Struct_metadata(const Struct_metadata& sm) { *this = sm; }
    Struct_metadata(Struct_metadata&& sm) { *this = std::move(sm); }
    Struct_metadata& operator=(const Struct_metadata& sm) { members = sm.members; }
    Struct_metadata& operator=(Struct_metadata&& sm) { members = std::move(sm.members); }
    ~Struct_metadata() {}
};

struct Struct_instance; // for "constructor" operator

struct Struct_type : CB_Type {
    CB_Owning_pointer<Struct_metadata> metadata = alloc<Struct_metadata>(Struct_metadata());

    Struct_type() : CB_Type{} {}
    Struct_type(const std::string& name) : CB_Type{name} {}
    template<typename T, typename Type=CB_Type*, Type=&T::type>
    Struct_type(const std::string& name, T&& default_value) : CB_Type{name, default_value} {}

    operator CB_Type() { return *this; }
    Struct_instance operator()(); // "constructor operator", creates an instance of this struct_type

    std::string toS() const override {
        ASSERT(metadata != nullptr);
        std::ostringstream oss;

        const std::string& name = typenames[uid];
        if (name == "") oss << "anonymous struct_type("+std::to_string(uid)+")";
        else oss << "struct " << name;

        oss << " { ";
        for (int i = 0; i < metadata->members.size; ++i) {
            Struct_member& member_metadata = metadata->members[i];
            oss << member_metadata.id.toS() << ":";
            oss << member_metadata.type.toS() << "=";
            if (member_metadata.explicit_uninitialized) oss << "---";
            else oss << member_metadata.default_value.toS();
            oss << "; ";
        }
        oss << "}";
        return oss.str();
    }

    bool operator==(const Struct_type& o) const { return (CB_Type)*this == (CB_Type)o; } // different struct types are all different
    bool operator!=(const Struct_type& o) const { return !(*this==o); }
};


struct Struct_instance {
    static CB_Type type; // Type "Struct_instance" - only here to conform to standard for a CB value. Special cases are needed elsewhere, to use struct_type instead for type checking.
    Struct_type struct_type;

    CB_Dynamic_seq<CB_Any> values;

    Struct_instance() {};
    Struct_instance(const Struct_type& struct_type) : struct_type{struct_type} { init_members(); };

    // copy and move constructors, destructor
    Struct_instance(const Struct_instance& si) : struct_type{si.struct_type} { *this = si; }
    Struct_instance(Struct_instance&& si) : struct_type{std::move(si.struct_type)} { *this = std::move(si); }
    Struct_instance& operator=(const Struct_instance& si) { ASSERT(struct_type == si.struct_type); values = si.values; }
    Struct_instance& operator=(Struct_instance&& si) { ASSERT(struct_type == si.struct_type); values = std::move(si.values); }
    ~Struct_instance() {}

    int get_CB_size() { ASSERT(false, "NYI"); } // TODO: implement

    bool is_same_type(const CB_Any& any, const CB_Type& type) const {
        // compare the type of an any with a CB_Type
        // if the any is of a struct type, compare its value's struct_type
        // else, compare its v_type

        if (any.v_type == Struct_instance::type) {
            // the value is a struct, so we should compare struct types
            return (CB_Type)any.value<Struct_instance>().struct_type == type;
        }
        else return any.v_type == type;
    }

    std::string toS() const {
        ASSERT(struct_type.metadata != nullptr);
        ASSERT(values.size == struct_type.metadata->members.size);
        std::ostringstream oss;

        const std::string& type_name = struct_type.typenames[struct_type.uid];
        if (type_name == "") oss << "instance of anonymous struct_type("+std::to_string(struct_type.uid)+")";
        else oss << type_name;

        oss << "(";
        for (int i = 0; i < values.size; ++i) {
            if (i > 0) oss << " ";
            Struct_member& member_metadata = struct_type.metadata->members[i];
            oss << member_metadata.id.toS() << ":";
            oss << member_metadata.type.toS() << "=";
            if (member_metadata.explicit_uninitialized) oss << "---";
            else {
                // get the current value
                ASSERT(is_same_type(values[i], member_metadata.type));
                oss << values[i].toS();
            }

            oss << ";";
        }
        oss << ")";
        return oss.str();
    }

    void init_members() {
        ASSERT(struct_type.metadata != nullptr);
        values.clear();
        for (auto& member : struct_type.metadata->members) {
            values.add(member.default_value);
        }
    }

    CB_Any& member(const CB_String& name) {
        ASSERT(struct_type.metadata != nullptr);
        ASSERT(values.size == struct_type.metadata->members.size);

        CB_Sharing_pointer<Struct_instance> used_struct_with_member = nullptr;

        for (int i = 0; i <  values.size; ++i) {
            Struct_member& member_metadata = struct_type.metadata->members[i];
            if (member_metadata.id == name) {
                ASSERT(is_same_type(values[i], member_metadata.type));
                return values[i];
            }

            // If a struct member is marked with the 'using' keyword, its members can also be reached.
            if (member_metadata.is_using && values[i].v_type == Struct_instance::type) {
                if (values[i].value<Struct_instance>().has_member(name)) {
                    ASSERT(used_struct_with_member == nullptr, "Ambiguous struct member: "+name.toS());
                    used_struct_with_member = &values[i].value<Struct_instance>();
                }
            }
        }

        if (used_struct_with_member != nullptr) return used_struct_with_member->member(name);

        ASSERT(false, "Cannot find member "+name.toS()+" in "+toS()); // log_error?
    }

    bool has_member(const CB_String& name) const {
        ASSERT(struct_type.metadata != nullptr);
        ASSERT(values.size == struct_type.metadata->members.size);
        for (int i = 0; i <  values.size; ++i) {
            const Struct_member& member_metadata = struct_type.metadata->members[i];
            if (member_metadata.id == name) return true;

            // If a struct member is marked with the 'using' keyword, its members can also be reached.
            if (member_metadata.is_using && values[i].v_type == Struct_instance::type) {
                auto member = values[i].value<Struct_instance>();
                if (member.has_member(name)) return true;
            }
        }
        return false;
    }
};

*/

