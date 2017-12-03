
/*
All static members has to have a place in memory.
That is, they have to be defined in a .cpp-file.

This file also contains
*/

#include "type.h"
#include "any.h"

std::map<int, std::string> CB_Type::typenames{};
std::map<int, CB_Any> CB_Type::default_values{};
CB_Type CB_Type::type = CB_Type("type", CB_Type("void")); // if we want a default value, this line has to be after the default_values initialization line
CB_Type CB_Any::type = CB_Type("any");

CB_Any CB_Type::default_value() const {
    CB_Any& any = default_values[uid];
    ASSERT(any.v_type == *this, "Type '"+toS()+"' has no default value!");
    return default_values[uid];
}

#include "primitives.h"

CB_Type CB_Bool::type = CB_Type("bool", CB_Bool());

#define CB_NUMBER_TYPE_STATIC_MEMBERS(T, CPP_t)              \
CB_Type CB_##T::type = CB_Type(#T, CB_##T());                \
const CB_##T CB_##T::MIN_VALUE = std::numeric_limits<CPP_t>::min();\
const CB_##T CB_##T::MAX_VALUE = std::numeric_limits<CPP_t>::max();\

CB_NUMBER_TYPE_STATIC_MEMBERS(i8, int8_t);
CB_NUMBER_TYPE_STATIC_MEMBERS(i16, int16_t);
CB_NUMBER_TYPE_STATIC_MEMBERS(i32, int32_t);
CB_NUMBER_TYPE_STATIC_MEMBERS(i64, int64_t);

CB_NUMBER_TYPE_STATIC_MEMBERS(u8, uint8_t);
CB_NUMBER_TYPE_STATIC_MEMBERS(u16, uint16_t);
CB_NUMBER_TYPE_STATIC_MEMBERS(u32, uint32_t);
CB_NUMBER_TYPE_STATIC_MEMBERS(u64, uint64_t);

CB_NUMBER_TYPE_STATIC_MEMBERS(f32, float);
CB_NUMBER_TYPE_STATIC_MEMBERS(f64, double);

CB_Type CB_Int::type = CB_Type("int", CB_Int());
const CB_Int CB_Int::MIN_VALUE = std::numeric_limits<int64_t>::min();
const CB_Int CB_Int::MAX_VALUE = std::numeric_limits<int64_t>::max();

CB_Type CB_Uint::type = CB_Type("uint", CB_Uint());
const CB_Uint CB_Uint::MIN_VALUE = std::numeric_limits<uint64_t>::min();
const CB_Uint CB_Uint::MAX_VALUE = std::numeric_limits<uint64_t>::max();

CB_Type CB_Float::type = CB_Type("float", CB_Float());
const CB_Float CB_Float::MIN_VALUE = std::numeric_limits<double>::min();
const CB_Float CB_Float::MAX_VALUE = std::numeric_limits<double>::max();

#include "range.h"
CB_Type CB_Range::type = CB_Type("range", CB_Range());

#include "string.h"
CB_Type CB_String::type = CB_Type("string", CB_String(""));

#include "flag.h"
CB_Type CB_Flag::type = CB_Type("flag");

#include "struct.h"
CB_Type Struct_instance::type = CB_Type("Struct_instance");
Struct_instance Struct_type::operator()() { return Struct_instance(*this); }

#include "function.h"
CB_Type CB_Function_type::type = CB_Type("function_type", CB_Function_type()); // if we want a default value, this line has to be after the default_values initialization line
CB_Type CB_Function::type = CB_Type("fn", CB_Function());

#include "../abstx/statements/scope.h"
CB_Type CB_Scope::type = CB_Type("scope", CB_Scope());





