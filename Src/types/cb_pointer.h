#pragma once

#include "cb_type.h"

#include <iomanip>

namespace Cube {

struct CB_Pointer : CB_Type
{
    static constexpr void* _default_value = nullptr;
    Shared<const CB_Type> v_type = nullptr;
    bool owning = false;

    CB_Pointer(bool explicit_unresolved=false) { uid = type->uid; if (explicit_unresolved) finalize(); }
    CB_Pointer(const std::string& name, size_t size, void const* default_value) : CB_Type(name, size, default_value) {}

    std::string toS() const override;

    bool is_primitive() const override { return true; }

    void finalize() override;

    void generate_typedef(std::ostream& os) const override;
    void generate_literal(std::ostream& os, void const* raw_data, uint32_t depth = 0) const override;
    void generate_destructor(std::ostream& os, const std::string& id, uint32_t depth = 0) const override;
};

}
