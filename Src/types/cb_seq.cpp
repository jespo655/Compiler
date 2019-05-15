#include "cb_seq.h"
#include "cb_primitives.h"
#include "cb_pointer.h"
#include "cb_any.h"
#include "../parser/token.h"


Shared<const CB_Type> CB_Seq::get_seq_type(Shared<const CB_Type> member_type) {
    Owned<CB_Seq> o = alloc(CB_Seq());
    o->v_type = member_type;
    o->finalize();
    return add_complex_cb_type(owned_static_cast<CB_Type>(std::move(o)));
}

std::string CB_Seq::toS() const {
    if (v_type == nullptr) return "_cb_unresolved_seq";
    std::ostringstream oss;
    v_type->generate_type(oss);
    // oss << v_type->toS(); // this doesn't work for sequences of structs that contains sequences of itself
    oss << "[]";
    return oss.str();
}

void CB_Seq::finalize() {
    std::string tos = toS();
    for (const auto& tn_pair : typenames) {
        if (tn_pair.second == tos) {
            // found existing sequence type with the same signature -> grab its id
            uid = tn_pair.first;
            return;
        }
    }
    // no matching signature found -> register new type
    register_type(tos, sizeof(_default_value), &_default_value);
}

void CB_Seq::generate_type(std::ostream& os) const {
    if (v_type == nullptr) os << toS();
    else {
        os << "_cb_seq_of";
        v_type->generate_type(os);
    }
}

void CB_Seq::generate_typedef(std::ostream& os) const {
    ASSERT(v_type != nullptr);
    os << "typedef struct { ";
    CB_u32::type->generate_type(os);
    os << " size; ";
    CB_u32::type->generate_type(os);
    os << " capacity; ";
    v_type->generate_type(os);
    os << "* v_ptr; } ";
    generate_type(os);
    os << ";" << std::endl;
}

void CB_Seq::generate_literal(std::ostream& os, void const* raw_data, const Token_context& context, uint32_t depth) const {
    ASSERT(raw_data);
    uint8_t const* raw_it = (uint8_t const*)raw_data;
    os << "(";
    generate_type(os);
    os << "){";
    CB_u32::type->generate_literal(os, raw_it, context, depth+1);
    raw_it += CB_u32::type->cb_sizeof();
    os << ", ";
    CB_u32::type->generate_literal(os, raw_it, context, depth+1);
    raw_it += CB_u32::type->cb_sizeof();
    os << ", ";
    CB_Pointer().generate_literal(os, raw_it, context, depth+1);
    os << "}";
}

void CB_Seq::generate_destructor(std::ostream& os, const std::string& id, const Token_context& context, uint32_t depth) const {
    if (depth > MAX_ALLOWED_DEPTH) { post_circular_reference_error(context); return; }
    CB_u32::type->generate_destructor(os, id+".size", context);
    CB_u32::type->generate_destructor(os, id+".capacity", context);
    os << "if (" << id << ".v_ptr) for(";
    CB_u32::type->generate_type(os);
    os << " _it=0; _it<" << id << ".capacity; ++i) { ";
    v_type->generate_destructor(os, id+".v_ptr[_it]", context, depth+1);
    os << " }" << std::endl;
    os << "free " << id << ".v_ptr;" << std::endl;
}

void CB_Seq::generate_for(std::ostream& os, const std::string& id, const std::string& it_name, uint64_t step, bool reverse, bool protected_scope) const {
    ASSERT(v_type);
    uint64_t it_uid = get_unique_id();
    if (!protected_scope) os << "{ "; // open brace to put unique iterator name out of scope for the rest of the program
    // variable
    v_type->generate_type(os);
    if (!v_type->is_primitive()) os << " const*";
    os << " " << it_name << "; ";
    // for start, unique it name
    os << "for (size_t _it_" << it_uid << " = ";
    if(reverse) os << id << ".size-1";
    else os << "0";
    os << "; ";
    // array indexing
    os << it_name << " = ";
    if (!v_type->is_primitive()) os << "&";
    generate_index_start(os, id);
    os << "_it_" << it_uid;
    generate_index_end(os);
    // stop condition
    os << ", _it_" << it_uid;
    if(reverse) os << " >= 0";
    else os << " < " << id << ".size";
    // increment/decrement
    os << "; _it_" << it_uid << (reverse?" -= ":" += ") << step << ")";
}
void CB_Seq::generate_for_after_scope(std::ostream& os, bool protected_scope) const {
    if (!protected_scope) os << "}" << std::endl; // close the brace with unique iterator name
}

void CB_Seq::generate_index_start(std::ostream& os, const std::string& id) const {
    os << id << ".v_ptr[";
}
void CB_Seq::generate_index_end(std::ostream& os) const {
    os << "]";
}

















Shared<const CB_Type> CB_Fixed_seq::get_seq_type(Shared<const CB_Type> member_type, uint32_t size) {
    Owned<CB_Fixed_seq> o = alloc<CB_Fixed_seq>(member_type, size);
    return add_complex_cb_type(owned_static_cast<CB_Type>(std::move(o)));
}

std::string CB_Fixed_seq::toS() const {
    if (v_type == nullptr) return "_cb_unresolved_fseq";
    std::ostringstream oss;
    v_type->generate_type(oss);
    // oss << v_type->toS(); // this doesn't work for sequences of structs that contains sequences of itself
    oss << "[";
    CB_u32::type->generate_literal(oss, &size, INVALID_CONTEXT);
    oss << "]";
    return oss.str();
}

void CB_Fixed_seq::finalize() {
    ASSERT((v_type == nullptr) ^ (size != 0), "fseq equivalence (no type <==> no size) must hold");

    // set default_value
    if (size != 0) {
        size_t member_size = v_type->cb_sizeof();
        _default_value_ptr = malloc(member_size * size);
        uint8_t* dv_it = (uint8_t*)_default_value_ptr;
        for (int i = 0; i < size; ++i) {
            memcpy(dv_it, v_type->default_value().v_ptr, member_size);
            dv_it += member_size;
        }
    } else {
        _default_value_ptr = malloc(0);
    }

    // check for existing types
    std::string tos = toS();
    for (const auto& tn_pair : typenames) {
        if (tn_pair.second == tos) {
            // found existing sequence type with the same signature -> grab its id
            uid = tn_pair.first;
            return;
        }
    }

    // no matching signature found -> register new type
    register_type(tos, v_type ? v_type->cb_sizeof() * size : 0, _default_value_ptr);
}

void CB_Fixed_seq::generate_type(std::ostream& os) const {
    if (v_type == nullptr) os << toS();
    else {
        os << "_cb_fseq_" << size << "_of";
        v_type->generate_type(os);
    }
}

void CB_Fixed_seq::generate_typedef(std::ostream& os) const {
    ASSERT(v_type != nullptr);
    os << "typedef struct { ";
    v_type->generate_type(os);
    os << " a[";
    CB_u32::type->generate_literal(os, &size, INVALID_CONTEXT);
    os << "]; } ";
    generate_type(os);
    os << ";" << std::endl;
}

void CB_Fixed_seq::generate_literal(std::ostream& os, void const* raw_data, const Token_context& context, uint32_t depth) const {
    ASSERT(raw_data != nullptr);
    uint8_t const* raw_it = (uint8_t const*)raw_data;
    os << "(";
    generate_type(os);
    os << "){";
    for (size_t i = 0; i < size; ++i) {
        if (i) os << ", ";
        v_type->generate_literal(os, raw_it, context, depth+1);
        raw_it += v_type->cb_sizeof();
    }
    os << "}";
}

void CB_Fixed_seq::generate_destructor(std::ostream& os, const std::string& id, const Token_context& context, uint32_t depth) const {
    if (depth > MAX_ALLOWED_DEPTH) { post_circular_reference_error(context); return; }
    os << "for(";
    CB_u32::type->generate_type(os);
    os << " _it=0; _it<";
    CB_u32::type->generate_literal(os, &size, context, depth+1);
    os << "; ++i) { ";
    v_type->generate_destructor(os, id+".a[_it]", context, depth+1);
    os << " }" << std::endl;
}

void CB_Fixed_seq::generate_for(std::ostream& os, const std::string& id, const std::string& it_name, uint64_t step, bool reverse, bool protected_scope) const {
    ASSERT(v_type);
    uint64_t it_uid = get_unique_id();
    if(!protected_scope) os << "{ "; // open brace to put unique iterator name out of scope for the rest of the program
    // variable
    v_type->generate_type(os);
    if (!v_type->is_primitive()) os << " const*";
    os << " " << it_name << "; ";
    // for start, unique it name
    os << "for (size_t _it_" << it_uid << " = ";
    if(reverse) os << size << "-1";
    else os << "0";
    os << "; ";
    // array indexing
    os << it_name << " = ";
    if (!v_type->is_primitive()) os << "&";
    generate_index_start(os, id);
    os << "_it_" << it_uid;
    generate_index_end(os);
    // stop condition
    os << ", _it_" << it_uid;
    if(reverse) os << " >= 0";
    else os << " < " << size;
    // increment/decrement
    os << "; _it_" << it_uid << (reverse?" -= ":" += ") << step << ")";
}
void CB_Fixed_seq::generate_for_after_scope(std::ostream& os, bool protected_scope) const {
    if(!protected_scope) os << "}" << std::endl; // close the brace with unique iterator name
}

void CB_Fixed_seq::generate_index_start(std::ostream& os, const std::string& id) const {
    os << id << ".a[";
}
void CB_Fixed_seq::generate_index_end(std::ostream& os) const {
    os << "]";
}


