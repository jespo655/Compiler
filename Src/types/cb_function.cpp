#include "cb_function.h"



std::string0 CB_Function::toS() const override {
    std::ostringstream oss;

    oss << "fn(";
    for (int i = 0; i < in_types.size; ++i) {
        if (i > 0) oss << ", ";
        oss << in_types[i]->toS();
    }
    oss << ")";
    if (out_types.size == 0) return oss.str();

    oss << "->";
    if (out_types.size > 1) oss << "(";
    for (int i = 0; i < out_types.size; ++i) {
        if (i > 0) oss << ", ";
        oss << out_types[i]->toS();
    }
    if (out_types.size > 1) oss << ")";
    return oss.str();
}


void CB_Function::finalize() override {
    std::string tos = toS();
    for (const auto& tn_pair : typenames) {
        if (tn_pair.second == tos) {
            // found existing function type with the same signature -> grab its id
            uid = tn_pair.first;
            return;
        }
    }
    // no matching signature found -> register new type
    register_type(tos, sizeof(_default_value), &_default_value);
}

void CB_Function::generate_typedef(ostream& os) const override {
    os << "typedef void(*";
    generate_type(os);
    os << ")(";
    for (int i = 0; i < in_types.size; ++i) {
        if (i) os << ", ";
        in_types[i]->generate_type(os);
        if (!in_types[i]->is_primitive()) os << " const*";
    }
    if (in_types.size > 0 && out_types.size > 0) os << ", ";
    for (int i = 0; i < out_types.size; ++i) {
        if (i) os << ", ";
        out_types[i]->generate_type(os);
        os << "*";
    }
    os << ");" << std::endl;
}

void CB_Function::generate_literal(ostream& os, void const* raw_data, uint32_t depth = 0) const override {
    if (!raw_data) os << "NULL";
    else if (!*(void**)raw_data) os << "NULL";
    else {
        os << std::hex << *(void**)raw_data;
        ASSERT(false, "warning: pointers are not the same outside compile time");
    }
}