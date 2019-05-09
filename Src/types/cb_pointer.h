#pragma once

#include "cb_type.h"

#include <iomanip>

struct CB_Pointer : CB_Type
{
    typedef void* c_typedef;
    static constexpr void* _default_value = nullptr;
    Shared<const CB_Type> v_type = nullptr;
    bool owning = false;

    CB_Pointer(Shared<const CB_Type> v_type=nullptr, bool owning=false) : v_type{v_type}, owning{owning} { finalize(); }
    CB_Pointer(const std::string& name, size_t size, void const* default_value) : CB_Type(name, size, default_value) {}

    std::string toS() const override;

    bool is_primitive() const override { return true; }

    void finalize() override;

    void generate_typedef(std::ostream& os) const override;
    void generate_literal(std::ostream& os, void const* raw_data, uint32_t depth = 0) const override;
    void generate_destructor(std::ostream& os, const std::string& id, uint32_t depth = 0) const override;
};
