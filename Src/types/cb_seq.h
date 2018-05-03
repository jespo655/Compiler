#pragma once

#include "cb_type.h"
#include "cb_primitives.h"
#include "cb_range.h"
#include "../utilities/unique_id.h"

/*
CB_Seq: a dynamic sequence that stores the elements on the heap

Syntax:
a : T[] = [T, size=N: t1, t2, t3]; // T inferred from members, N evaluated compile time, must be a positive integer.
a : T[] = [T: t1, t2, t3]; // N inferred by the number of arguments
a : T[] = [size=N: t1, t2, t3]; // T inferred from the type of the members
a : T[] = [t1, t2, t3]; // T and N inferred
*/

struct CB_Indexable {
    virtual void generate_index_start(ostream& os, const std::string& id) const = 0;
    virtual void generate_index_end(ostream& os) const = 0;
};


struct CB_Seq : CB_Type, CB_Iterable, CB_Indexable
{
    struct c_representation { uint32_t size; uint32_t capacity; void* v_ptr; }; // void* is actually T*
    static constexpr c_representation _default_value = (c_representation){0, 0, nullptr};
    Shared<const CB_Type> v_type = nullptr;

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

    bool is_primitive() const override { return false; }

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

    void generate_typedef(ostream& os) const override {
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
    void generate_literal(ostream& os, void const* raw_data, uint32_t depth = 0) const override {
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
        os << "}";
    }
    void generate_destructor(ostream& os, const std::string& id, uint32_t depth = 0) const override {
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

    void generate_for(ostream& os, const std::string& id, const std::string& it_name = "it", uint64_t step = 1, bool reverse = false, bool protected_scope = true) const override {
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
    void generate_for_after_scope(ostream& os, bool protected_scope = true) const override {
        if (!protected_scope) os << "}" << std::endl; // close the brace with unique iterator name
    }

    void generate_index_start(ostream& os, const std::string& id) const override {
        os << id << ".v_ptr[";
    }
    void generate_index_end(ostream& os) const override {
        os << "]";
    }

};




struct CB_Fixed_seq : CB_Type, CB_Iterable, CB_Indexable
{
    Shared<const CB_Type> v_type = nullptr;
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

    bool is_primitive() const override { return false; }

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


    void generate_typedef(ostream& os) const override {
        ASSERT(v_type != nullptr);
        os << "typedef ";
        v_type->generate_type(os);
        os << "[";
        CB_u32::type->generate_literal(os, &size);
        os << "] ";
        generate_type(os);
        os << ";" << std::endl;
    }
    void generate_literal(ostream& os, void const* raw_data, uint32_t depth = 0) const override {
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
    void generate_destructor(ostream& os, const std::string& id, uint32_t depth = 0) const override {
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

    void generate_for(ostream& os, const std::string& id, const std::string& it_name = "it", uint64_t step = 1, bool reverse = false, bool protected_scope = true) const override {
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
    void generate_for_after_scope(ostream& os, bool protected_scope = true) const override {
        if(!protected_scope) os << "}" << std::endl; // close the brace with unique iterator name
    }

    void generate_index_start(ostream& os, const std::string& id) const override {
        os << id << "[";
    }
    void generate_index_end(ostream& os) const override {
        os << "]";
    }
};




