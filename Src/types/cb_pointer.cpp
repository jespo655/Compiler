#include "cb_pointer.h"

namespace Cube {

std::string CB_Pointer::toS() const {
    if (v_type == nullptr) return "_cb_unresolved_pointer";
    std::ostringstream oss;
    v_type->generate_type(oss);
    // oss << v_type->toS(); // this doesn't work for pointers to structs that contains pointers to itself
    oss << (owning?"*!":"*");
    return oss.str();
}

void CB_Pointer::finalize() {
    std::string tos = toS();
    for (const auto& tn_pair : typenames) {
        if (tn_pair.second == tos) {
            // found existing pointer type with the same signature -> grab its id
            uid = tn_pair.first;
            return;
        }
    }
    // no matching signature found -> register new type
    register_type(tos, sizeof(_default_value), &_default_value);
}


void CB_Pointer::generate_typedef(std::ostream& os) const {
    os << "typedef ";
    v_type->generate_type(os);
    os << "* ";
    generate_type(os);
    os << ";" << std::endl;
}
void CB_Pointer::generate_literal(std::ostream& os, void const* raw_data, uint32_t depth) const {
    ASSERT(raw_data);
    if (!*(void**)raw_data) os << "NULL";
    os << std::hex << *(void**)raw_data;
}
void CB_Pointer::generate_destructor(std::ostream& os, const std::string& id, uint32_t depth) const {
    if (depth > MAX_ALLOWED_DEPTH) { post_circular_reference_error(); return; }
    if (owning) {
        v_type->generate_destructor(os, "*"+id, depth+1);
        os << "free " << id << ";" << std::endl;
    }
}

}
