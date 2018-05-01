
/*
All static members has to have a place in memory.
That is, they have to be defined in a .cpp-file.
*/

#include "cb_type.h"
#include "cb_any.h"

std::map<int, std::string> CB_Type::typenames{};
std::map<int, any> CB_Type::default_values{};
std::map<int, size_t> CB_Type::cb_sizes{};

const bool CB_Type::primitive;
constexpr uint32_t CB_Type::_default_value;
static const CB_Type static_cb_type("type", sizeof(CB_Type::_default_value), &CB_Type::_default_value);
const shared<const CB_Type> CB_Type::type = &static_cb_type;

const bool CB_Any::primitive;
constexpr void* CB_Any::_default_value;
static const CB_Any static_cb_any("any", sizeof(CB_Any::_default_value), &CB_Any::_default_value);
const shared<const CB_Type> CB_Any::type = &static_cb_any;

const any& CB_Type::default_value() const
{
    const any& a = default_values[uid];
    if (*a.v_type != *this) {
        std::cerr << std::endl << "error: default value type doesn't match: dv.v_type.uid = "
            << a.v_type->uid << "; *this.uid = " << this->uid << std::endl;
    }
    ASSERT(*a.v_type == *this, "Type '"+toS()+"' has no default value!");
    return a;
}

void CB_Type::register_type(const std::string& name, size_t size, void const* default_value)
{
    uid = get_unique_type_id();
    ASSERT(typenames[uid] == ""); // can't register double value (this should never happen if uid works correctly)
    typenames[uid] = name;
    cb_sizes[uid] = size;
    default_values[uid] = std::move(any(this, default_value));
}


// Below: static type instances for types that never change

#include "cb_primitives.h"

#define PRIMITIVE_STATICS(cpp_type, tos, c_type) \
const bool cpp_type::primitive; \
constexpr c_type cpp_type::_default_value; \
static const cpp_type static_##cpp_type(tos, sizeof(cpp_type::_default_value), &cpp_type::_default_value); \
const shared<const CB_Type> cpp_type::type = &static_##cpp_type;

PRIMITIVE_STATICS(CB_Bool, "bool", bool);

PRIMITIVE_STATICS(CB_i8, "i8", int8_t);
PRIMITIVE_STATICS(CB_i16, "i16", int16_t);
PRIMITIVE_STATICS(CB_i32, "i32", int32_t);
PRIMITIVE_STATICS(CB_i64, "i64", int64_t);

PRIMITIVE_STATICS(CB_u8, "u8", uint8_t);
PRIMITIVE_STATICS(CB_u16, "u16", uint16_t);
PRIMITIVE_STATICS(CB_u32, "u32", uint32_t);
PRIMITIVE_STATICS(CB_u64, "u64", uint64_t);

PRIMITIVE_STATICS(CB_f32, "f32", float);
PRIMITIVE_STATICS(CB_f64, "f64", double);

PRIMITIVE_STATICS(CB_Int, "int", int64_t);
PRIMITIVE_STATICS(CB_Uint, "uint", uint64_t);
PRIMITIVE_STATICS(CB_Float, "float", double);

PRIMITIVE_STATICS(CB_Flag, "flag", uint8_t);

#include "cb_range.h"
const bool CB_Range::primitive;
constexpr double CB_Range::_default_value[2];
static const CB_Range static_cb_range("range", sizeof(CB_Range::_default_value), &CB_Range::_default_value);
const shared<const CB_Type> CB_Range::type = &static_cb_range;

#include "cb_string.h"
const bool CB_String::primitive;
constexpr char CB_String::_default_str[];
constexpr char const* CB_String::_default_value;
static const CB_String static_cb_string("string", sizeof(&CB_String::_default_value), &CB_String::_default_value);
const shared<const CB_Type> CB_String::type = &static_cb_string;

#include "cb_pointer.h"
const bool CB_Pointer::primitive;
constexpr void* CB_Pointer::_default_value;
static const CB_Pointer _unresolved_pointer = CB_Pointer(true);
// type is registered with CB_Pointer::finalize()

#include "cb_function.h"
const bool CB_Function ::primitive;
constexpr void(*CB_Function::_default_value)();
// type is registered with CB_Function::finalize()

#include "cb_struct.h"
const bool CB_Struct::primitive;
// default value is different for each instance
// type is registered with CB_Struct::finalize()



// #include "../abstx/statements/scope.h"
// CB_Type CB_Scope::type = CB_Type("scope", 0, CB_Scope());




#ifdef TEST

#include "cb_struct.h" // @todo

void test_type(shared<const CB_Type> type)
{
    std::cout << type->toS() << ": ";
    type->generate_type(std::cout);
    std::cout << ", uid: " << type->uid << ", size: " << type->cb_sizeof()
        << ", defval: ";
    type->generate_literal(std::cout, type->default_value().v_ptr);
    std::cout << std::endl;
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

    { CB_String type; test_type(&type); }
    { CB_u8 type; test_type(&type); }
    { CB_Range type; test_type(&type); }
    { CB_Flag type; test_type(&type); }
    { CB_Flag type; test_type(&type); }

    { CB_Function type; type.finalize(); test_type(&type); }
    { CB_Function type; type.finalize(); test_type(&type); }
    { CB_Function type; type.in_types.add(CB_Bool::type); type.finalize(); test_type(&type); }
    { CB_Function type; type.out_types.add(CB_Bool::type); type.finalize(); test_type(&type); }

    { CB_Struct type; type.add_member("i", CB_Int::type); type.finalize(); test_type(&type); }
    { CB_Struct type; type.add_member("i", CB_Int::type); type.finalize(); test_type(&type); }
    { CB_Struct type; type.add_member("i", CB_Int::type); type.finalize(); test_type(&type); }
    { CB_Struct type; type.add_member("s", CB_String::type); type.finalize(); test_type(&type); }

    {
        CB_Struct type; type.add_member("s", CB_String::type); type.finalize();
        CB_Struct type2 = type;
        test_type(&type);
        test_type(&type2);
        CB_Struct type3 = std::move(type);
        test_type(&type3);
        // test_type(&type);
    }

    {
        CB_Struct type;
        type.add_member("a", CB_i8::type);
        type.add_member("b", CB_i16::type);
        type.add_member("c", CB_i16::type);
        type.add_member("d", CB_i16::type);
        type.finalize();
        test_type(&type);
    }

    { CB_Pointer type; type.v_type = CB_Int::type; type.finalize(); test_type(&type); }
    { CB_Pointer type; type.v_type = CB_String::type; type.finalize(); test_type(&type); }
    { CB_Pointer type; type.v_type = CB_Range::type; type.finalize(); test_type(&type); }
    {
        CB_Pointer type; type.v_type = CB_Range::type; type.finalize(); test_type(&type);
        CB_Pointer pt1; pt1.v_type = &type; pt1.owned=true; pt1.finalize(); test_type(&pt1);
        CB_Pointer pt2; pt2.v_type = &pt1;  pt2.owned=false; pt2.finalize(); test_type(&pt2);
        CB_Pointer pt3; pt3.v_type = &pt2;  pt3.owned=true; pt3.finalize(); test_type(&pt3);
        CB_Pointer pt4; pt4.v_type = &pt3;  pt4.owned=true; pt4.finalize(); test_type(&pt4);
        CB_Pointer pt5; pt5.v_type = &pt4;  pt5.owned=true; pt5.finalize(); test_type(&pt5);
        pt5.generate_destructor(std::cout, "ptr");
    }

    {
        CB_Struct type;
        CB_Pointer pt1; pt1.finalize(); // forward declaration / finalize as unresolved_pointer
        // CB_Pointer pt1; pt1.v_type = CB_Int::type; pt1.owned=false; pt1.finalize();
        CB_Pointer pt2; pt2.v_type = &pt1; pt2.owned=true; pt2.finalize();
        type.add_member("sp", &pt1);
        type.add_member("op", &pt2);
        type.finalize();
        pt1.v_type = &type; pt1.owned=false; pt1.finalize(); // final finalize now when type is finalized
        test_type(&type);
        test_type(&pt1);
        type.generate_destructor(std::cout, "s");
    }

    {
        CB_Struct s1, s2, s3;
        CB_Pointer pt1; pt1.finalize(); // forward declaration / finalize as unresolved_pointer
        s1.add_member("s3op", &pt1); s1.finalize();
        s2.add_member("s1", &s1); s2.finalize();
        s3.add_member("s2", &s2); s3.finalize();
        pt1.v_type = &s3; pt1.owned=true; pt1.finalize(); // final finalize now when type is finalized
        // s1.generate_literal(std::cout, s1._default_value); std::cout << std::endl;
        s2.generate_destructor(std::cout, "s"); std::cout << std::endl;
    }

    CB_String::type->generate_literal(std::cout, CB_String::type->default_value().v_ptr) << std::endl;

// TODO: CB_Seq, (CB_Set)

}

#endif
