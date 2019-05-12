#include "cb_seq.h"
#include "cb_primitives.h"
#include "cb_pointer.h"
#include "cb_any.h"


Shared<const CB_Type> CB_Seq::get_seq_type(Shared<const CB_Type> member_type) {
    Owned<CB_Seq> o = alloc(CB_Seq());
    o->v_type = member_type;
    o->finalize();
    return add_complex_cb_type(owned_static_cast<CB_Type>(std::move(o)));
}

std::string CB_Seq::toS() const {
    if (v_type == nullptr) return "_cb_unresolved_sequence";
    std::ostringstream oss;
    v_type->generate_type(oss);
    // oss << v_type->toS(); // this doesn't work for sequences of structs that contains sequences of itself
    oss << "[]";
    return oss.str();
}

void CB_Seq::finalize() {
    std::string tos = toS();
    std::cout << "seq with v_type " << v_type << " has tos " << tos << std::endl;

    for (const auto& tn_pair : typenames) {
        if (tn_pair.second == tos) {
            // found existing sequence type with the same signature -> grab its id
            std::cout << "found existing sequence type with the same signature -> grab its id" << std::endl;
            uid = tn_pair.first;
            return;
        }
    }
    // no matching signature found -> register new type
    register_type(tos, sizeof(_default_value), &_default_value);
    std::cout << "finalized seq with v_type " << v_type << " as ";
    generate_type(std::cout);
    std::cout << std::endl;
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
void CB_Seq::generate_literal(std::ostream& os, void const* raw_data, uint32_t depth) const {
    ASSERT(raw_data);
    uint8_t const* raw_it = (uint8_t const*)raw_data;
    os << "(";
    generate_type(os);
    os << "){";
    CB_u32::type->generate_literal(os, raw_it, depth+1);
    raw_it += CB_u32::type->cb_sizeof();
    os << ", ";
    CB_u32::type->generate_literal(os, raw_it, depth+1);
    raw_it += CB_u32::type->cb_sizeof();
    os << ", ";
    CB_Pointer().generate_literal(os, raw_it, depth+1);
    os << "}";
}
void CB_Seq::generate_destructor(std::ostream& os, const std::string& id, uint32_t depth) const {
    if (depth > MAX_ALLOWED_DEPTH) { post_circular_reference_error(); return; }
    CB_u32::type->generate_destructor(os, id+".size");
    CB_u32::type->generate_destructor(os, id+".capacity");
    os << "if (" << id << ".v_ptr) for(";
    CB_u32::type->generate_type(os);
    os << " _it=0; _it<" << id << ".capacity; ++i) { ";
    v_type->generate_destructor(os, id+".v_ptr[_it]", depth+1);
    os << " }" << std::endl;
    os << "free " << id << ".v_ptr;" << std::endl;
}

void CB_Seq::generate_for(std::ostream& os, const std::string& id, const std::string& it_name, uint64_t step, bool reverse, bool protected_scope) const {
    uint64_t it_uid = get_unique_id();
    if (!protected_scope) os << "{ "; // open brace to put unique iterator name out of scope for the rest of the program
    // variable
    v_type->generate_type(os);
    if (!v_type->is_primitive()) os << " const*";
    os << " " << it_name << ";";
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
    os << "; _it_" << uid << (reverse?" -= ":" += ") << step << ")";
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
    Owned<CB_Fixed_seq> o = alloc(CB_Fixed_seq());
    o->v_type = member_type;
    o->size = size;
    o->finalize();
    return add_complex_cb_type(owned_static_cast<CB_Type>(std::move(o)));
}

std::string CB_Fixed_seq::toS() const {
    if (v_type == nullptr) return "_cb_unresolved_sequence";
    std::ostringstream oss;
    v_type->generate_type(oss);
    // oss << v_type->toS(); // this doesn't work for sequences of structs that contains sequences of itself
    oss << "[";
    CB_u32::type->generate_literal(oss, &size);
    oss << "]";
    return oss.str();
}

void CB_Fixed_seq::finalize() {
    // set default_value
    if (size != 0) {
        ASSERT(v_type != nullptr);
        size_t member_size = v_type->cb_sizeof();
        _default_value = malloc(member_size * size);
        uint8_t* dv_it = (uint8_t*)_default_value;
        for (int i = 0; i < size; ++i) {
            memcpy(dv_it, v_type->default_value().v_ptr, member_size);
            dv_it += member_size;
        }
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
    ASSERT(v_type != nullptr);
    register_type(tos, v_type->cb_sizeof() * size, _default_value);
}


void CB_Fixed_seq::generate_typedef(std::ostream& os) const {
    ASSERT(v_type != nullptr);
    os << "typedef ";
    v_type->generate_type(os);
    os << "[";
    CB_u32::type->generate_literal(os, &size);
    os << "] ";
    generate_type(os);
    os << ";" << std::endl;
}
void CB_Fixed_seq::generate_literal(std::ostream& os, void const* raw_data, uint32_t depth) const {
    ASSERT(raw_data != nullptr);
    uint8_t const* raw_it = (uint8_t const*)raw_data;
    os << "{";
    for (size_t i = 0; i < size; ++i) {
        if (i) os << ", ";
        v_type->generate_literal(os, raw_it);
        raw_it += v_type->cb_sizeof();
    }
    os << "}";
}
void CB_Fixed_seq::generate_destructor(std::ostream& os, const std::string& id, uint32_t depth) const {
    if (depth > MAX_ALLOWED_DEPTH) { post_circular_reference_error(); return; }
    CB_u32::type->generate_destructor(os, id+".size");
    CB_u32::type->generate_destructor(os, id+".capacity");
    os << "if (" << id << ".v_ptr) for(";
    CB_u32::type->generate_type(os);
    os << " _it=0; _it<" << id << ".capacity; ++i) { ";
    v_type->generate_destructor(os, id+".v_ptr[_it]", depth+1);
    os << " }" << std::endl;
    os << "free " << id << ".v_ptr;" << std::endl;
}

void CB_Fixed_seq::generate_for(std::ostream& os, const std::string& id, const std::string& it_name, uint64_t step, bool reverse, bool protected_scope) const {
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
    os << "; _it_" << uid << (reverse?" -= ":" += ") << step << ")";
}
void CB_Fixed_seq::generate_for_after_scope(std::ostream& os, bool protected_scope) const {
    if(!protected_scope) os << "}" << std::endl; // close the brace with unique iterator name
}

void CB_Fixed_seq::generate_index_start(std::ostream& os, const std::string& id) const {
    os << id << "[";
}
void CB_Fixed_seq::generate_index_end(std::ostream& os) const {
    os << "]";
}


