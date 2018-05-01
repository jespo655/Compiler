#pragma once

#include "cb_type.h"

#include <iomanip>

struct CB_Pointer : CB_Type
{
    static const bool primitive = true;
    static constexpr void* _default_value = nullptr;
    shared<const CB_Type> v_type = nullptr;
    bool owning = false;

    CB_Pointer(bool explicit_unresolved=false) { uid = type->uid; if (explicit_unresolved) finalize(); }
    CB_Pointer(const std::string& name, size_t size, void const* default_value) : CB_Type(name, size, default_value) {}

    std::string toS() const override {
        if (v_type == nullptr) return "_cb_unresolved_pointer";
        std::ostringstream oss;
        v_type->generate_type(oss);
        // oss << v_type->toS(); // this doesn't work for pointers to structs that contains pointers to itself
        oss << (owning?"*!":"*");
        return oss.str();
    }

    void finalize() {
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

    virtual ostream& generate_typedef(ostream& os) const override {
        os << "typedef ";
        v_type->generate_type(os);
        os << "* ";
        generate_type(os);
        return os << ";";
    }
    virtual ostream& generate_literal(ostream& os, void const* raw_data, uint32_t depth = 0) const override {
        ASSERT(raw_data);
        if (!*(void**)raw_data) return os << "NULL";
        return os << std::hex << *(void**)raw_data;
    }
    virtual ostream& generate_destructor(ostream& os, const std::string& id, uint32_t depth = 0) const override {
        if (depth > MAX_ALLOWED_DEPTH) { post_circular_reference_error(); return os; }
        if (owning) {
            v_type->generate_destructor(os, "*"+id, depth+1);
            return os << "free " << id << ";" << std::endl;
        }
    }

};


