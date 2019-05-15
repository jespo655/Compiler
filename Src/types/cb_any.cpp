#include "cb_any.h"
#include "cb_string.h"
#include "cb_primitives.h"
#include "../parser/token.h"

void CB_Any::generate_type(std::ostream& os) const
{
    os << "_cb_any";
}

void CB_Any::generate_typedef(std::ostream& os) const {
    os << "typedef struct { ";
    CB_Type::type->generate_type(os);
    os << " type; void* v_ptr; } ";
    generate_type(os);
    os << ";" << std::endl;
}
void CB_Any::generate_literal(std::ostream& os, void const* raw_data, const Token_context& context, uint32_t depth) const {
    if (depth > MAX_ALLOWED_DEPTH) { post_circular_reference_error(context); os << "void"; return; }
    ASSERT(raw_data);
    os << "(";
    generate_type(os);
    os << "){";
    CB_Type::type->generate_literal(os, raw_data, context, depth+1);
    uint8_t const* raw_it = (uint8_t const*)raw_data;
    raw_it += CB_Type::type->cb_sizeof();
    os << ", " << std::hex << (void**)raw_it << "}";
}
void CB_Any::generate_destructor(std::ostream& os, const std::string& id, const Token_context& context, uint32_t depth) const {
    if (depth > MAX_ALLOWED_DEPTH) { post_circular_reference_error(context); return; }
}

std::string Any::toS() const  {
    if (v_ptr) {
        ASSERT(v_type);
        std::ostringstream oss;
        v_type->generate_literal(oss, v_ptr, INVALID_CONTEXT);
        return oss.str();
    }
    else return "---";
}

Any& Any::operator=(const Any& Any) {
    if (this != &Any) {
        v_ptr = Any.v_ptr;
        v_type = Any.v_type;
    }
    return *this;
}

Any& Any::operator=(Any&& Any) {
    if (this != &Any) {
        v_type = Any.v_type;
        v_ptr = Any.v_ptr;
        Any.v_ptr = nullptr;
    }
    return *this;
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
