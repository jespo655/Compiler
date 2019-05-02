#include "cb_any.h"

namespace Cube {

void CB_Any::generate_type(std::ostream& os) const override
{
    os << "_cb_any";
}

void CB_Any::generate_typedef(std::ostream& os) const override {
    os << "typedef struct { ";
    CB_Type::type->generate_type(os);
    os << " type; void* v_ptr; } ";
    generate_type(os);
    os << ";" << std::endl;
}
void CB_Any::generate_literal(std::ostream& os, void const* raw_data, uint32_t depth = 0) const override {
    if (depth > MAX_ALLOWED_DEPTH) { post_circular_reference_error(); os << "void"; return; }
    ASSERT(raw_data);
    os << "(";
    generate_type(os);
    os << "){";
    CB_Type::type->generate_literal(os, raw_data, depth+1);
    uint8_t const* raw_it = (uint8_t const*)raw_data;
    raw_it += CB_Type::type->cb_sizeof();
    os << ", " << std::hex << (void**)raw_it << "}";
}
void CB_Any::generate_destructor(std::ostream& os, const std::string& id, uint32_t depth = 0) const override {
    if (depth > MAX_ALLOWED_DEPTH) { post_circular_reference_error(); return; }
}


CB_Type::c_typedef parse_type_id(const Any& any) {
    ASSERT(*any.v_type == *CB_Type::type);
    ASSERT(any.v_ptr);
    return *(CB_Type::c_typedef*)any.v_ptr;
}

Shared<const CB_Type> parse_type(const Any& any) {
    return get_built_in_type(parse_type_id(any));
}

std::string parse_string(const Any& any) {
    ASSERT(*any.v_type == *CB_String::type);
    ASSERT(any.v_ptr);
    ASSERT(*(char**)any.v_ptr);
    return std::string(*(char**)any.v_ptr);
}

int64_t parse_int(const Any& any) {
    ASSERT(any.v_ptr);
    if (*any.v_type == *CB_i64::type) return *(int64_t*)any.v_ptr;
    if (*any.v_type == *CB_i32::type) return *(int32_t*)any.v_ptr;
    if (*any.v_type == *CB_i16::type) return *(int16_t*)any.v_ptr;
    if (*any.v_type == *CB_i8::type) return *(int8_t*)any.v_ptr;
    ASSERT(*any.v_type == *CB_Int::type);
    return *(int64_t*)any.v_ptr;
}

uint64_t parse_uint(const Any& any) {
    ASSERT(any.v_ptr);
    if (*any.v_type == *CB_u64::type) return *(uint64_t*)any.v_ptr;
    if (*any.v_type == *CB_u32::type) return *(uint32_t*)any.v_ptr;
    if (*any.v_type == *CB_u16::type) return *(uint16_t*)any.v_ptr;
    if (*any.v_type == *CB_u8::type) return *(uint8_t*)any.v_ptr;
    ASSERT(*any.v_type == *CB_Uint::type);
    return *(uint64_t*)any.v_ptr;
}

uint64_t parse_float(const Any& any) {
    ASSERT(any.v_ptr);
    if (*any.v_type == *CB_f64::type) return *(double*)any.v_ptr;
    if (*any.v_type == *CB_f32::type) return *(float*)any.v_ptr;
    ASSERT(*any.v_type == *CB_Float::type);
    return *(double*)any.v_ptr;
}

}