#include "type_checker.h"
#include <memory>
#include "error_handler.h"
#include "assert.h"
#include <vector>
#include <string>

using namespace std;




const Type_info undefined_type = Type_info("",0);

Identifier::Identifier() : name{""}, type{undefined_type}, value_ptr{nullptr} {}

Identifier::Identifier(const std::string& s, const Type_info& t)
    : name{s}, type{t}, value_ptr{t.size==0?nullptr:malloc(t.size)}
{}

Identifier::~Identifier()
{
    free(value_ptr);
}

Identifier::Identifier(const Identifier& o)
    : name{o.name}, type{o.type}, value_ptr{o.type.size==0?nullptr:malloc(o.type.size)}
{
    memcpy(value_ptr, o.value_ptr, o.type.size);
}

Identifier::Identifier(Identifier&& o)
    : name{std::move(o.name)}, type{std::move(o.type)}, value_ptr{o.value_ptr}
{
    o.value_ptr = nullptr;
}

Identifier& Identifier::operator=(const Identifier& o)
{
    if (type.size != o.type.size) {
        free(value_ptr);
        value_ptr = malloc(o.type.size);
    }
    name = o.name;
    type = o.type;
    memcpy(value_ptr, o.value_ptr, o.type.size);
}
Identifier& Identifier::operator=(Identifier&& o)
{
    free(value_ptr);
    name = std::move(o.name);
    type = std::move(o.type);
    value_ptr = o.value_ptr;
    o.value_ptr = nullptr;
}









typedef signed char i8 ;
typedef signed short i16 ;
typedef signed int i32 ;
typedef signed long long i64 ;

typedef unsigned char u8 ;
typedef unsigned short u16 ;
typedef unsigned int u32 ;
typedef unsigned long long u64 ;


vector<Type_info> types
{
    { "int", sizeof(i32) }, // the same as i32
    { "i8", sizeof(i8) },
    { "i16", sizeof(i16) },
    { "i32", sizeof(i32) },
    { "i64", sizeof(i64) },
    { "u8", sizeof(u8) },
    { "u16", sizeof(u16) },
    { "u32", sizeof(u32) },
    { "u64", sizeof(u64) },
    { "float", sizeof(float) }, // maybe f32 and f64?
    { "double", sizeof(double) },
    { "dump", 0 }, // can be the assignment target of any type. The result is discarded
    { "struct", sizeof(void*) },
    { "void", 0 },
    { "type", 0 },
    // // TODO:
    // { "string", sizeof(u8*) }, // for now: u8*
    // { "pointer", sizeof(void*) },
    // { "typed_pointer", 2*sizeof(void*) }, // also contains type information
};


Type_info get_type_info(const string& type_name)
{
    if (type_name == "") return undefined_type;
    for (Type_info& type : types)
    {
        if (type.type == type_name) {
            return type;
        }
    }
    // the type does not exist in the global scope.
    return Type_info(type_name,0);
    // TODO: add the type to the local scope. For this we need to know the size of the type first.
}

Type_info get_type_info(const Token& token)
{
    return get_type_info(token.token);
}

