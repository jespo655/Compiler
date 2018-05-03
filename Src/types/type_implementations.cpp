
/*
All static members has to have a place in memory.
That is, they have to be defined in a .cpp-file.
*/

#include "cb_type.h"
#include "cb_any.h"

std::map<int, std::string> CB_Type::typenames{};
std::map<int, Any> CB_Type::default_values{};
std::map<int, size_t> CB_Type::cb_sizes{};

constexpr uint32_t CB_Type::_default_value;
static const CB_Type static_cb_type("type", sizeof(CB_Type::_default_value), &CB_Type::_default_value);
const Shared<const CB_Type> CB_Type::type = &static_cb_type;

constexpr void* CB_Any::_default_value;
static const CB_Any static_cb_any("any", sizeof(CB_Any::_default_value), &CB_Any::_default_value);
const Shared<const CB_Type> CB_Any::type = &static_cb_any;

const Any& CB_Type::default_value() const
{
    const Any& a = default_values[uid];
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
    default_values[uid] = std::move(Any(this, default_value));
}


// Below: static type instances for types that never change

#include "cb_primitives.h"

#define PRIMITIVE_STATICS(cpp_type, tos, c_type) \
constexpr c_type cpp_type::_default_value; \
static const cpp_type static_##cpp_type(tos, sizeof(cpp_type::_default_value), &cpp_type::_default_value); \
const Shared<const CB_Type> cpp_type::type = &static_##cpp_type;

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
constexpr double CB_Range::_default_value[2];
static const CB_Range static_cb_range("range", sizeof(CB_Range::_default_value), &CB_Range::_default_value);
const Shared<const CB_Type> CB_Range::type = &static_cb_range;

#include "cb_string.h"
constexpr char CB_String::_default_str[];
constexpr char const* CB_String::_default_value;
static const CB_String static_cb_string("string", sizeof(&CB_String::_default_value), &CB_String::_default_value);
const Shared<const CB_Type> CB_String::type = &static_cb_string;

#include "cb_pointer.h"
constexpr void* CB_Pointer::_default_value;
// an explicit unresolved_pointer is needed just so that unresolved pointers can be used and thrown away without storing
// otherwise, the next time an unresolved pointer is used, the reference will be a dangling pointer (which is bad)
static const CB_Pointer _unresolved_pointer = CB_Pointer(true);
// type is registered with CB_Pointer::finalize()

#include "cb_function.h"
constexpr void(*CB_Function::_default_value)();
// type is registered with CB_Function::finalize()

#include "cb_struct.h"
// default value is different for each instance
// type is registered with CB_Struct::finalize()

#include "cb_seq.h"
constexpr CB_Seq::c_representation CB_Seq::_default_value;
// an explicit unresolved_sequence is needed just so that unresolved sequences can be used and thrown away without storing
// otherwise, the next time an unresolved sequence is used, the reference will be a dangling pointer (which is bad)
static const CB_Seq _unresolved_sequence = CB_Seq(true);
// type is registered with CB_Seq::finalize()

// an explicit unresolved_sequence is needed just so that unresolved sequences can be used and thrown away without storing
// otherwise, the next time an unresolved sequence is used, the reference will be a dangling pointer (which is bad)
static const CB_Fixed_seq _unresolved_fixed_sequence = CB_Fixed_seq(true);
// type is registered with CB_Fixed_seq::finalize()




// #include "../abstx/statements/scope.h"
// CB_Type Abstx_scope::type = CB_Type("scope", 0, Abstx_scope());


Seq<Shared<const CB_Type>> built_in_types;

void build_static_type_seq() {
    if (built_in_types.size > 0) return; // already built
    built_in_types.add(&static_cb_type);
    built_in_types.add(&static_cb_any);
    built_in_types.add(&static_CB_Bool);
    built_in_types.add(&static_CB_i8);
    built_in_types.add(&static_CB_i16);
    built_in_types.add(&static_CB_i32);
    built_in_types.add(&static_CB_i64);
    built_in_types.add(&static_CB_u8);
    built_in_types.add(&static_CB_u16);
    built_in_types.add(&static_CB_u32);
    built_in_types.add(&static_CB_u64);
    built_in_types.add(&static_CB_f32);
    built_in_types.add(&static_CB_f64);
    built_in_types.add(&static_CB_Int);
    built_in_types.add(&static_CB_Uint);
    built_in_types.add(&static_CB_Float);
    built_in_types.add(&static_CB_Flag);
    built_in_types.add(&static_cb_range);
    built_in_types.add(&static_cb_string);
    // only primitives - seq, set, function types and struct types are not included here
}

// slower, but more generic
Shared<const CB_Type> get_built_in_type(const std::string& name)
{
    build_static_type_seq();
    for (auto& type : built_in_types) {
        // these two alternatives should be equivalent, but a map lookup should be faster
        if (name == CB_Type::typenames[type->uid]) return type;
        // if (name == type->toS()) return type;
    }
}

// faster, but not as useful
Shared<const CB_Type> get_built_in_type(uint32_t uid)
{
    build_static_type_seq();
    for (auto& type : built_in_types) {
        if (type->uid == uid) return type;
    }
}

Shared<Seq<Shared<const CB_Type>>> get_built_in_types()
{
    build_static_type_seq();
    return &built_in_types;
}







#ifdef TEST

void test_type(Shared<const CB_Type> type)
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
        CB_Pointer pt1; pt1.v_type = &type; pt1.owning=true; pt1.finalize(); test_type(&pt1);
        CB_Pointer pt2; pt2.v_type = &pt1;  pt2.owning=false; pt2.finalize(); test_type(&pt2);
        CB_Pointer pt3; pt3.v_type = &pt2;  pt3.owning=true; pt3.finalize(); test_type(&pt3);
        CB_Pointer pt4; pt4.v_type = &pt3;  pt4.owning=true; pt4.finalize(); test_type(&pt4);
        CB_Pointer pt5; pt5.v_type = &pt4;  pt5.owning=true; pt5.finalize(); test_type(&pt5);
        pt5.generate_destructor(std::cout, "ptr");
    }

    {
        CB_Struct type;
        CB_Pointer pt1; pt1.finalize(); // forward declaration / finalize as unresolved_pointer
        // CB_Pointer pt1; pt1.v_type = CB_Int::type; pt1.owning=false; pt1.finalize();
        CB_Pointer pt2; pt2.v_type = &pt1; pt2.owning=true; pt2.finalize();
        type.add_member("sp", &pt1);
        type.add_member("op", &pt2);
        type.finalize();
        pt1.v_type = &type; pt1.owning=false; pt1.finalize(); // final finalize now when type is finalized
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
        pt1.v_type = &s3; pt1.owning=true; pt1.finalize(); // final finalize now when type is finalized
        // s1.generate_literal(std::cout, s1._default_value); std::cout << std::endl;
        // s2.generate_destructor(std::cout, "s"); std::cout << std::endl; // this should give cyclic reference error
    }

    // @TODO: (CB_Set)

    { CB_Seq s; s.v_type = CB_Int::type; s.finalize(); test_type(&s); }
    { CB_Fixed_seq s; s.v_type = CB_Int::type; s.size = 5; s.finalize(); test_type(&s); }
    { CB_Fixed_seq s; s.v_type = CB_Int::type; s.size = 5; s.finalize(); test_type(&s); }
    { CB_Fixed_seq s; s.v_type = CB_Int::type; s.size = 6; s.finalize(); test_type(&s); }
    { CB_Fixed_seq s; s.v_type = CB_i64::type; s.size = 5; s.finalize(); test_type(&s); }

}

#endif
