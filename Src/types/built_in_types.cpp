
/*
All static members has to have a place in memory.
That is, they have to be defined in a .cpp-file.
*/

#include "cb_type.h"
#include "cb_any.h"
#include "cb_range.h"
#include "cb_string.h"
#include "cb_pointer.h"
#include "cb_function.h"
#include "cb_struct.h"
#include "cb_seq.h"
#include "cb_primitives.h"

std::map<CB_Type::c_typedef, std::string> CB_Type::typenames{};
std::map<CB_Type::c_typedef, Any> CB_Type::default_values{};
std::map<CB_Type::c_typedef, size_t> CB_Type::cb_sizes{};

std::map<CB_Type::c_typedef, Shared<const CB_Type>> CB_Type::built_in_types{};
std::map<CB_Type::c_typedef, Owned<CB_Type>> CB_Type::complex_types{};

constexpr CB_Type::c_typedef CB_Type::_default_value;
static const CB_Type static_cb_type("type", sizeof(CB_Type::_default_value), &CB_Type::_default_value);
const Shared<const CB_Type> CB_Type::type = &static_cb_type;

constexpr void* CB_Any::_default_value;
static const CB_Any static_cb_any("any", sizeof(CB_Any::_default_value), &CB_Any::_default_value);
const Shared<const CB_Type> CB_Any::type = &static_cb_any;

#define PRIMITIVE_STATICS(cpp_type, tos) \
constexpr cpp_type::c_typedef cpp_type::_default_value; \
static const cpp_type static_##cpp_type(tos, sizeof(cpp_type::_default_value), &cpp_type::_default_value); \
const Shared<const CB_Type> cpp_type::type = &static_##cpp_type;

PRIMITIVE_STATICS(CB_Bool, "bool");

PRIMITIVE_STATICS(CB_i8, "i8");
PRIMITIVE_STATICS(CB_i16, "i16");
PRIMITIVE_STATICS(CB_i32, "i32");
PRIMITIVE_STATICS(CB_i64, "i64");

PRIMITIVE_STATICS(CB_u8, "u8");
PRIMITIVE_STATICS(CB_u16, "u16");
PRIMITIVE_STATICS(CB_u32, "u32");
PRIMITIVE_STATICS(CB_u64, "u64");

PRIMITIVE_STATICS(CB_f32, "f32");
PRIMITIVE_STATICS(CB_f64, "f64");

PRIMITIVE_STATICS(CB_Int, "int");
PRIMITIVE_STATICS(CB_Uint, "uint");
PRIMITIVE_STATICS(CB_Float, "float");

PRIMITIVE_STATICS(CB_Flag, "flag");

constexpr int64_t CB_Range::_default_value[2];
static const CB_Range static_cb_range("range", sizeof(CB_Range::_default_value), &CB_Range::_default_value);
const Shared<const CB_Type> CB_Range::type = &static_cb_range;

constexpr double CB_Float_range::_default_value[2];
static const CB_Float_range static_cb_float_range("range", sizeof(CB_Float_range::_default_value), &CB_Float_range::_default_value);
const Shared<const CB_Type> CB_Float_range::type = &static_cb_range;

constexpr char CB_String::_default_str[];
constexpr char const* CB_String::_default_value;
static const CB_String static_cb_string("string", sizeof(&CB_String::_default_value), &CB_String::_default_value);
const Shared<const CB_Type> CB_String::type = &static_cb_string;

constexpr void* CB_Pointer::_default_value;
// an explicit unresolved_pointer is needed just so that unresolved pointers can be used and thrown away without storing
// otherwise, the next time an unresolved pointer is used, the reference will be a dangling pointer (which is bad)
static const CB_Pointer _unresolved_pointer = CB_Pointer(true);
// type is registered with CB_Pointer::finalize()

constexpr void(*CB_Function::_default_value)();
// type is registered with CB_Function::finalize()

// default value is different for each instance
// type is registered with CB_Struct::finalize()

constexpr CB_Seq::c_representation CB_Seq::_default_value;
// an explicit unresolved_sequence is needed just so that unresolved sequences can be used and thrown away without storing
// otherwise, the next time an unresolved sequence is used, the reference will be a dangling pointer (which is bad)
static const CB_Seq _unresolved_sequence = CB_Seq(true);
// type is registered with CB_Seq::finalize()

// an explicit unresolved_sequence is needed just so that unresolved sequences can be used and thrown away without storing
// otherwise, the next time an unresolved sequence is used, the reference will be a dangling pointer (which is bad)
static const CB_Fixed_seq _unresolved_fixed_sequence = CB_Fixed_seq(true);
// type is registered with CB_Fixed_seq::finalize()




//
// CB_Type Abstx_scope::type = CB_Type("scope", 0, Abstx_scope());



Shared<const CB_Type> add_complex_cb_type(Owned<CB_Type>&& type) {
    ASSERT(type); // can't add nullptr
    LOG("adding complex type " << type.toS());
    CB_Type::c_typedef uid = type->uid;
    Shared<CB_Type> p = CB_Type::complex_types[uid];
    if (p != nullptr) return p;
    CB_Type::complex_types[uid] = std::move(type);
    return (Shared<CB_Type>)CB_Type::complex_types[uid]; // double implicit cast is too hard
}

void prepare_built_in_types() {
    if (!CB_Type::built_in_types.empty()) return; // already built
    CB_Type::built_in_types[static_cb_type.uid] = &static_cb_type;
    CB_Type::built_in_types[static_cb_any.uid] = &static_cb_any;
    CB_Type::built_in_types[static_CB_Bool.uid] = &static_CB_Bool;
    CB_Type::built_in_types[static_CB_i8.uid] = &static_CB_i8;
    CB_Type::built_in_types[static_CB_i16.uid] = &static_CB_i16;
    CB_Type::built_in_types[static_CB_i32.uid] = &static_CB_i32;
    CB_Type::built_in_types[static_CB_i64.uid] = &static_CB_i64;
    CB_Type::built_in_types[static_CB_u8.uid] = &static_CB_u8;
    CB_Type::built_in_types[static_CB_u16.uid] = &static_CB_u16;
    CB_Type::built_in_types[static_CB_u32.uid] = &static_CB_u32;
    CB_Type::built_in_types[static_CB_u64.uid] = &static_CB_u64;
    CB_Type::built_in_types[static_CB_f32.uid] = &static_CB_f32;
    CB_Type::built_in_types[static_CB_f64.uid] = &static_CB_f64;
    CB_Type::built_in_types[static_CB_Int.uid] = &static_CB_Int;
    CB_Type::built_in_types[static_CB_Uint.uid] = &static_CB_Uint;
    CB_Type::built_in_types[static_CB_Float.uid] = &static_CB_Float;
    CB_Type::built_in_types[static_CB_Flag.uid] = &static_CB_Flag;
    CB_Type::built_in_types[static_cb_range.uid] = &static_cb_range;
    CB_Type::built_in_types[static_cb_float_range.uid] = &static_cb_float_range;
    CB_Type::built_in_types[static_cb_string.uid] = &static_cb_string;
    // only primitives - seq, set, function types and struct types are not included here
}

// slower, but more generic
Shared<const CB_Type> get_built_in_type(const std::string& name)
{
    prepare_built_in_types();
    for (auto& type : CB_Type::built_in_types) {
        // these two alternatives should be equivalent, but a map lookup should be faster
        if (name == CB_Type::typenames[type.second->uid]) return type.second;
        // if (name == type.second->toS()) return type;
    }
    for (auto& type : CB_Type::complex_types) {
        if (name == CB_Type::typenames[type.second->uid]) return (Shared<CB_Type>)type.second; // double implicit cast is too hard
        // if (name == type.second->toS()) return type;
    }
    return nullptr;
}

// faster, but not as useful
Shared<const CB_Type> get_built_in_type(CB_Type::c_typedef uid)
{
    prepare_built_in_types();
    Shared<CB_Type> p = CB_Type::complex_types[uid];
    if (p) return p;
    return CB_Type::built_in_types[uid];
}

void generate_typedefs(std::ostream& os)
{
    // nullpointers are added to the map if we try to access a value that doesn't exist
    // don't generate types from nullpointers
    for (const auto& type : CB_Type::built_in_types) {
        if (type.second) type.second->generate_typedef(os);
    }
    for (const auto& type : CB_Type::complex_types) {
        if (type.second) type.second->generate_typedef(os);
    }
}



