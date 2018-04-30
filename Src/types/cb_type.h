#pragma once

#include "../utilities/assert.h"
#include "../utilities/unique_id.h"
#include "../utilities/pointers.h"

#include <map>
#include <string>
#include <ostream>
using std::ostream;

/*
Cube is a statically typed language.
This means that all types has to be known at compile time.
Generic types are allowed as long as the actual type can be determined at compiletime.

Types can also be stored and used for runtime type checking.
In this case they are represented as a 32 bit unsigned integer.

Type names in c++ are prefixed with CB_, to make it easy to see what is a type and what is not.
They can be instanciated and used like any c++ struct.

Each type has a static member CB_Type type, which holds the type identifier for that type.
Each type has a static default value, which can be accessed as CB_Any with CB_T::type.default_value().
Each type has a static flag primitive, which determines if an instance of the class should be passed by value or const pointer to functions

Data received from compile-time executed CB-code is received as void pointers. That data might then
    be used as a literal in the next compile step. Therefore, each type needs to know how to parse
    raw data and write it as a literal.





The built-in types in this language are:

primitives:
* CB_Type - a type
* CB_Function_type - a subclass of CB_Type; The value of an CB_Function_type is determined by its in and out parameters.
* CB_Struct_type - a subclass of CB_Type; each struct definition creates its own struct type.

* CB_Owning_pointer - holds a pointer to a value and deletes it when the destructor is called
* CB_Sharing_pointer - holds a pointer to a value but does not delete it
* CB_Flag - integer type that holds exactly one bit in a bit field

* CB_bool - a boolean value (8 bit)

* CB_i8 - a 8 bit signed integer
* CB_i16 - a 16 bit signed integer
* CB_i32 - a 32 bit signed integer
* CB_i64 - a 64 bit signed integer
* CB_int - a signed integer of unspecified size (target specific)

* CB_u8 - a 8 bit unsigned integer
* CB_u16 - a 16 bit unsigned integer
* CB_u32 - a 32 bit unsigned integer
* CB_u64 - a 64 bit unsigned integer
* CB_uint - an usigned integer of unspecified size (target specific)

* CB_f32 - a 32 bit floating point value (single precision)
* CB_f64 - a 64 bit floating point value (double precision)
* CB_float - a floating point value of unspecified size (target specific)

non-primitives:
* CB_Dynamic_seq - holds a dynamicly sized array of CB values - similar to std::vector
* CB_Static_seq - holds a statically sized array of CB values - similar to std::array
* CB_Any - holds a pointer to any value in a type safe manner
* CB_Function - holds a function pointer
* CB_Range - holds a start and an end value (both f64) for a range
* CB_String - holds a null-terminated UTF-8 string and its length - similar to std::string
* CB_Struct - holds a byte array containing data members

C++ representations of types are important in this language because of the #run directive:

a :: #run foo();

The function gets compiled and executed, and the result has to be stored in a c++ representation of the data.
Then that same data has to be able to be outputted as a C style literal.

*/


struct any; // used for default values

struct CB_Type
{
    static const shared<const CB_Type> type; // self reference / CB_Type
    static const bool primitive = true;
    static constexpr uint32_t _default_value = 0;
    static std::map<int, std::string> typenames; // mapped from uid to name. Only compile time.
    static std::map<int, any> default_values; // mapped from uid to value. Only compile time.
    static std::map<int, size_t> cb_sizes; // mapped from uid to size. Only compile time.

    uint32_t uid;

    CB_Type() { uid = type->uid; } // default value for the type
    CB_Type(const std::string& name, size_t size, void const* default_value) {
        register_type(name, size, default_value); // new type
    }
    virtual ~CB_Type() {}
    void register_type(const std::string& name, size_t size, void const* default_value);

    virtual std::string toS() const {
        const std::string& name = typenames[uid];
        if (name == "") return "type_"+std::to_string(uid);
        return name;
    }

    virtual size_t alignment() const { return cb_sizeof(); }

    const any& default_value() const;
    size_t cb_sizeof() const { return cb_sizes[uid]; }

    bool operator==(const CB_Type& o) const { return uid == o.uid; }
    bool operator!=(const CB_Type& o) const { return !(*this==o); }

    static int get_unique_type_id() {
        static int id=0; // -1 is uninitialized
        ASSERT(id >= 0); // if id is negative then the int has looped around. More than INT_MAX unique types should never be needed.
        return id++;
    }

    // code generation functions
    virtual ostream& generate_type(ostream& os) const { return os << "_cb_type_" << uid; }
    virtual ostream& generate_typedef(ostream& os) const {
        os << "typedef uint32_t ";
        generate_type(os);
        return os << ";";
    }
    virtual ostream& generate_literal(ostream& os, void const* raw_data) const { ASSERT(raw_data); return os << *(uint32_t*)raw_data << "UL"; }
    virtual ostream& generate_destructor(ostream& os, const std::string& id) const { return os; };
    // constructor:
    //   type name = literal(default_value); // default
    //   type name; // explicit uninitialized

};

// #include "any.h" // any requires complete definition of CB_Type, so we have to include this here

