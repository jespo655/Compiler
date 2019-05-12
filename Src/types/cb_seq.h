#pragma once

#include "cb_type.h"
#include "cb_range.h"
#include "../utilities/unique_id.h"
#include "../utilities/pointers.h"

/*
CB_Seq: a dynamic sequence that stores the elements on the heap

Syntax:
a : T[] = [T, size=N: t1, t2, t3]; // T inferred from members, N evaluated compile time, must be a positive integer.
a : T[] = [T: t1, t2, t3]; // N inferred by the number of arguments
a : T[] = [size=N: t1, t2, t3]; // T inferred from the type of the members
a : T[] = [t1, t2, t3]; // T and N inferred
*/

struct CB_Indexable {
    virtual void generate_index_start(std::ostream& os, const std::string& id) const = 0;
    virtual void generate_index_end(std::ostream& os) const = 0;
};

struct CB_Seq : CB_Type, CB_Iterable, CB_Indexable
{
    typedef struct { uint32_t size; uint32_t capacity; void* v_ptr; } c_typedef; // void* is actually T*
    static constexpr c_typedef _default_value = (c_typedef){0, 0, nullptr};
    Shared<const CB_Type> v_type = nullptr;

    CB_Seq(Shared<const CB_Type> v_type=nullptr) : v_type{v_type} { finalize(); }
    CB_Seq(const std::string& name, size_t size, void const* default_value) : CB_Type(name, size, default_value) {}

    static Shared<const CB_Type> get_seq_type(Shared<const CB_Type> member_type);

    std::string toS() const override;

    bool is_primitive() const override { return false; }

    void finalize() override;

    void generate_typedef(std::ostream& os) const override;
    void generate_literal(std::ostream& os, void const* raw_data, uint32_t depth = 0) const override;
    void generate_destructor(std::ostream& os, const std::string& id, uint32_t depth = 0) const override;

    void generate_for(std::ostream& os, const std::string& id, const std::string& it_name = "it", uint64_t step = 1, bool reverse = false, bool protected_scope = true) const override;
    void generate_for_after_scope(std::ostream& os, bool protected_scope = true) const override;

    void generate_index_start(std::ostream& os, const std::string& id) const override;
    void generate_index_end(std::ostream& os) const override;
};

struct CB_Fixed_seq : CB_Type, CB_Iterable, CB_Indexable
{
    typedef void* c_typedef; // actually T*
    Shared<const CB_Type> v_type = nullptr;
    c_typedef _default_value = nullptr;
    uint32_t size = 0;

    CB_Fixed_seq(Shared<const CB_Type> v_type=nullptr) : v_type{v_type} { finalize(); }
    CB_Fixed_seq(const std::string& name, size_t size, void const* default_value) : CB_Type(name, size, default_value) {}
    ~CB_Fixed_seq() { free(_default_value); }

    static Shared<const CB_Type> get_seq_type(Shared<const CB_Type> member_type, uint32_t size);

    std::string toS() const override;

    bool is_primitive() const override { return false; }

    void finalize() override;

    void generate_typedef(std::ostream& os) const override;
    void generate_literal(std::ostream& os, void const* raw_data, uint32_t depth = 0) const override;
    void generate_destructor(std::ostream& os, const std::string& id, uint32_t depth = 0) const override;

    void generate_for(std::ostream& os, const std::string& id, const std::string& it_name = "it", uint64_t step = 1, bool reverse = false, bool protected_scope = true) const override;
    void generate_for_after_scope(std::ostream& os, bool protected_scope = true) const override;

    void generate_index_start(std::ostream& os, const std::string& id) const override;
    void generate_index_end(std::ostream& os) const override;
};
