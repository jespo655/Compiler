
/*
All static members has to have a place in memory.
That is, they have to be defined in a .cpp-file.
*/

#include "cb_type.h"
#include "cb_any.h"

static const uint64_t ZERO_BYTES[] = {0, 0};
#define NULL_VALUE ZERO_BYTES[0]
#define ZERO_VALUE ZERO_BYTES[0]

std::map<int, std::string> CB_Type::typenames{};
std::map<int, any> CB_Type::default_values{};
std::map<int, size_t> CB_Type::cb_sizes{};
CB_Type CB_Type::type = CB_Type("type", sizeof(uint32_t), &NULL_VALUE); // if we want a default value, this line has to be after the default_values initialization line
CB_Type CB_Any::type = CB_Type("any", 0, &NULL_VALUE);

const any& CB_Type::default_value() const
{
    const any& a = default_values[uid];
    ASSERT(a.v_type == *this, "Type '"+toS()+"' has no default value!");
    return a;
}

void CB_Type::register_type(const std::string& name, size_t size, void const* default_value)
{
    uid = get_unique_type_id();
    ASSERT(typenames[uid] == ""); // can't register double value (this should never happen if uid works correctly)
    typenames[uid] = name;
    cb_sizes[uid] = size;
    default_values[uid] = std::move(any(*this, default_value));
}


// Below: static type instances for types that never change

#include "cb_primitives.h"

CB_Type CB_Bool::type = CB_Type("bool", sizeof(bool), &ZERO_VALUE);

CB_Type CB_i8::type = CB_Type("i8", sizeof(int8_t), &ZERO_VALUE);
CB_Type CB_i16::type = CB_Type("i16", sizeof(int16_t), &ZERO_VALUE);
CB_Type CB_i32::type = CB_Type("i32", sizeof(int32_t), &ZERO_VALUE);
CB_Type CB_i64::type = CB_Type("i64", sizeof(int64_t), &ZERO_VALUE);

CB_Type CB_u8::type = CB_Type("u8", sizeof(uint8_t), &ZERO_VALUE);
CB_Type CB_u16::type = CB_Type("u16", sizeof(uint16_t), &ZERO_VALUE);
CB_Type CB_u32::type = CB_Type("u32", sizeof(uint32_t), &ZERO_VALUE);
CB_Type CB_u64::type = CB_Type("u64", sizeof(uint64_t), &ZERO_VALUE);

CB_Type CB_f32::type = CB_Type("f32", sizeof(float), &ZERO_VALUE);
CB_Type CB_f64::type = CB_Type("f64", sizeof(double), &ZERO_VALUE);

CB_Type CB_Int::type = CB_Type("int", sizeof(int64_t), &ZERO_VALUE);
CB_Type CB_Uint::type = CB_Type("uint", sizeof(uint64_t), &ZERO_VALUE);
CB_Type CB_Float::type = CB_Type("float", sizeof(double), &ZERO_VALUE);

CB_Type CB_Flag::type = CB_Type("flag", sizeof(uint8_t), &ZERO_VALUE);

#include "cb_range.h"
CB_Type CB_Range::type = CB_Type("range", 2*sizeof(uint64_t), &ZERO_VALUE);

#include "cb_string.h"
CB_Type CB_String::type = CB_Type("string", sizeof(char*), &NULL_VALUE);

// not included: CB_Struct, CB_Function




// #include "../abstx/statements/scope.h"
// CB_Type CB_Scope::type = CB_Type("scope", 0, CB_Scope());




#ifdef TEST

#include "cb_struct.h" // @todo
#include "cb_function.h"

void test_type(const CB_Type& type)
{
    std::cout << type.toS() << ": ";
    type.generate_type(std::cout);
    std::cout << ", uid: " << type.uid << ", size: " << type.cb_sizeof() << std::endl;
}

int main()
{
    test_type(CB_Type::type);
    test_type(CB_i8::type);
    test_type(CB_i16::type);
    test_type(CB_i32::type);
    test_type(CB_i64::type);
    test_type(CB_Int::type);
    test_type(CB_u8::type);
    test_type(CB_u16::type);
    test_type(CB_u32::type);
    test_type(CB_u64::type);
    test_type(CB_Uint::type);
    test_type(CB_f32::type);
    test_type(CB_f64::type);
    test_type(CB_Float::type);
    test_type(CB_Bool::type);
    test_type(CB_Flag::type);
    test_type(CB_Range::type);
    test_type(CB_String::type);

    { CB_String type; test_type(type); }
    { CB_u8 type; test_type(type); }
    { CB_Range type; test_type(type); }
    { CB_Flag type; test_type(type); }
    { CB_Flag type; test_type(type); }

    { CB_Function type; type.finalize(); test_type(type); }
    { CB_Function type; type.finalize(); test_type(type); }
    { CB_Function type; type.in_types.add(&CB_Bool::type); type.finalize(); test_type(type); }
    { CB_Function type; type.out_types.add(&CB_Bool::type); type.finalize(); test_type(type); }

    { CB_Struct type; type.add_member("i", shared<CB_Type>(&CB_Int::type)); type.finalize(); test_type(type); }
    { CB_Struct type; type.add_member("i", shared<CB_Type>(&CB_Int::type)); type.finalize(); test_type(type); }
    { CB_Struct type; type.add_member("i", shared<CB_Type>(&CB_Int::type)); type.finalize(); test_type(type); }

}

#endif
