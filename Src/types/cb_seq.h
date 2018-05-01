#pragma once

#include "cb_type.h"
#include "cb_primitives.h"

/*
CB_Seq: a dynamic sequence that stores the elements on the heap

Syntax:
a : T[] = [T, size=N: t1, t2, t3]; // T inferred from members, N evaluated compile time, must be a positive integer.
a : T[] = [T: t1, t2, t3]; // N inferred by the number of arguments
a : T[] = [size=N: t1, t2, t3]; // T inferred from the type of the members
a : T[] = [t1, t2, t3]; // T and N inferred
*/



struct CB_Seq : CB_Type
{
    static const bool primitive = false;
    struct c_representation { uint32_t size; uint32_t capacity; void* v_ptr; }; // void* is actually T*
    static constexpr c_representation _default_value = (c_representation){0, 0, nullptr};
    shared<const CB_Type> v_type = nullptr;

    CB_Seq(bool explicit_unresolved=false) { uid = type->uid; if (explicit_unresolved) finalize(); }
    CB_Seq(const std::string& name, size_t size, void const* default_value) : CB_Type(name, size, default_value) {}

    std::string toS() const override {
        if (v_type == nullptr) return "_cb_unresolved_sequence";
        std::ostringstream oss;
        v_type->generate_type(oss);
        // oss << v_type->toS(); // this doesn't work for sequences of structs that contains sequences of itself
        oss << "[]";
        return oss.str();
    }

    void finalize() {
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

    virtual ostream& generate_typedef(ostream& os) const override {
        ASSERT(v_type != nullptr);
        os << "typedef struct { ";
        CB_u32::type->generate_type(os);
        os << " size; ";
        CB_u32::type->generate_type(os);
        os << " capacity; ";
        v_type->generate_type(os);
        os << "* v_ptr; } ";
        generate_type(os);
        return os << ";";
    }
    virtual ostream& generate_literal(ostream& os, void const* raw_data, uint32_t depth = 0) const override {
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
        CB_Pointer(true).generate_literal(os, raw_it, depth+1);
        return os << "}";
    }
    virtual ostream& generate_destructor(ostream& os, const std::string& id, uint32_t depth = 0) const override {
        if (depth > MAX_ALLOWED_DEPTH) { post_circular_reference_error(); return os; }
        CB_u32::type->generate_destructor(os, id+".size");
        CB_u32::type->generate_destructor(os, id+".capacity");
        os << "if (" << id << ".v_ptr) for(";
        CB_u32::type->generate_type(os);
        os << " _it=0; _it<" << id << ".capacity; ++i) { ";
        v_type->generate_destructor(os, id+".v_ptr[_it]", depth+1);
        os << " }" << std::endl;
        return os << "free " << id << ".v_ptr;" << std::endl;
    }

};




struct CB_Fixed_seq : CB_Type
{
    static const bool primitive = false;
    shared<const CB_Type> v_type = nullptr;
    void* _default_value = nullptr;
    uint32_t size = 0;

    CB_Fixed_seq(bool explicit_unresolved=false) { uid = type->uid; if (explicit_unresolved) finalize(); }
    CB_Fixed_seq(const std::string& name, size_t size, void const* default_value) : CB_Type(name, size, default_value) {}
    ~CB_Fixed_seq() { free(_default_value); }

    std::string toS() const override {
        if (v_type == nullptr) return "_cb_unresolved_sequence";
        std::ostringstream oss;
        v_type->generate_type(oss);
        // oss << v_type->toS(); // this doesn't work for sequences of structs that contains sequences of itself
        oss << "[";
        CB_u32::type->generate_literal(oss, &size);
        oss << "]";
        return oss.str();
    }

    void finalize() {
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


    virtual ostream& generate_typedef(ostream& os) const override {
        ASSERT(v_type != nullptr);
        os << "typedef ";
        v_type->generate_type(os);
        os << "[";
        CB_u32::type->generate_literal(os, &size);
        os << "] ";
        generate_type(os);
        return os << ";";
    }
    virtual ostream& generate_literal(ostream& os, void const* raw_data, uint32_t depth = 0) const override {
        ASSERT(raw_data != nullptr);
        uint8_t const* raw_it = (uint8_t const*)raw_data;
        os << "{";
        for (size_t i = 0; i < size; ++i) {
            if (i) os << ", ";
            v_type->generate_literal(os, raw_it);
            raw_it += v_type->cb_sizeof();
        }
        return os << "}";
    }
    virtual ostream& generate_destructor(ostream& os, const std::string& id, uint32_t depth = 0) const override {
        if (depth > MAX_ALLOWED_DEPTH) { post_circular_reference_error(); return os; }
        CB_u32::type->generate_destructor(os, id+".size");
        CB_u32::type->generate_destructor(os, id+".capacity");
        os << "if (" << id << ".v_ptr) for(";
        CB_u32::type->generate_type(os);
        os << " _it=0; _it<" << id << ".capacity; ++i) { ";
        v_type->generate_destructor(os, id+".v_ptr[_it]", depth+1);
        os << " }" << std::endl;
        return os << "free " << id << ".v_ptr;" << std::endl;
    }
};




