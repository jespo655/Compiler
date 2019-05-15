#pragma once

#include "../utilities/pointers.h"
#include "../utilities/debug.h"

#include <map>
#include <string>
#include <ostream>

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

struct Any; // used for default values
struct Token_context; // used for error messages in code generation

struct CB_Type : Serializable
{
    static const Shared<const CB_Type> type; // self reference / CB_Type
    typedef uint32_t c_typedef;
    static constexpr c_typedef _default_value = 0;
    static std::map<c_typedef, std::string> typenames; // mapped from uid to name. Only compile time.
    static std::map<c_typedef, Any> default_values; // mapped from uid to value. Only compile time.
    static std::map<c_typedef, size_t> cb_sizes; // mapped from uid to size. Only compile time.

    static std::map<c_typedef, Shared<const CB_Type>> built_in_types; // mapped from uid to shared type representation of built in types (filled in automatically, types are owned statically by cpp file)
    static std::map<c_typedef, Owned<CB_Type>> complex_types; // mapped from uid to owned type representation of complex type (filled in over time as the types are defined in the program)

    c_typedef uid;

    CB_Type() { uid = type->uid; } // default value for the type
    CB_Type(const std::string& name, size_t size, void const* default_value) {
        register_type(name, size, default_value); // new type
    }
    virtual ~CB_Type() {}
    void register_type(const std::string& name, size_t size, void const* default_value);

    virtual std::string toS() const override;

    // is_primitive(): should return true if we'd rather copy the value itself than a pointer to it (+do pointer dereferences!)
    virtual bool is_primitive() const { return true; }

    // alignment(): should return the minimum alignment according to c standard (1, 2, 4 or 8 (on 64bit) bytes)
    virtual size_t alignment() const { return cb_sizeof(); }

    // finalize(): ensure that the uid is correct (important for complex types that might get duplicated)
    virtual void finalize() { /* do nothing */ }

    const Any& default_value() const;
    size_t cb_sizeof() const { return cb_sizes[uid]; }

    bool operator==(const CB_Type& o) const { return uid == o.uid; }
    bool operator!=(const CB_Type& o) const { return !(*this==o); }

    static int get_unique_type_id();

    // code generation functions
    virtual void generate_type(std::ostream& os) const;
    virtual void generate_typedef(std::ostream& os) const;
    // literal & destructor has an additional argument depth, to safeguard against infinite loops
    virtual void generate_literal(std::ostream& os, void const* raw_data, const Token_context& context, uint32_t depth = 0) const;
    virtual void generate_destructor(std::ostream& os, const std::string& id, const Token_context& context, uint32_t depth = 0) const;
    // constructor:
    //   type name = literal(default_value); // default
    //   type name; // explicit uninitialized

    // If a circular reference loops more than MAX_ALLOWED_DEPTH number of times, you should call post_circular_reference_error(context)
    static const uint32_t MAX_ALLOWED_DEPTH = 1000;
    void post_circular_reference_error(const Token_context& context) const;

};


// functions to get build in types
Shared<const CB_Type> get_built_in_type(const std::string& name); // slower, but more generic
Shared<const CB_Type> get_built_in_type(CB_Type::c_typedef uid); // faster, but not as useful
void prepare_built_in_types(); // prepare list of built in types (done automatically in get_built_in_type)

// function to add complex built-in types (CB_Function, CB_Pointer, CB_Seq or CB_Struct)
Shared<const CB_Type> add_complex_cb_type(Owned<CB_Type>&& type);

template<typename T>
Shared<const T> add_complex_type(Owned<T>&& type) {
    type->finalize(); // just to be sure uid is correct
    Owned<CB_Type> cb_type = owned_static_cast<CB_Type>(std::move(type));
    Shared<const CB_Type> p = add_complex_cb_type(std::move(cb_type));
    return static_pointer_cast<const T>(p);
}

// function to generate typedef for all built-in types
void generate_typedefs(std::ostream& os);
