#include "cb_type.h"
#include "cb_any.h"
#include "../utilities/assert.h"
#include "../utilities/unique_id.h"
#include "../utilities/error_handler.h"
#include "../parser/token.h"

std::string CB_Type::toS() const {
    const std::string& name = typenames[uid];
    if (name == "") return "type_"+std::to_string(uid);
    return name;
}

// code generation functions
void CB_Type::generate_type(std::ostream& os) const {
    os << "_cb_type";
    if (uid != type->uid) os << "_" << uid;
}
void CB_Type::generate_typedef(std::ostream& os) const {
    os << "typedef uint32_t ";
    generate_type(os);
    os << ";" << std::endl;
}

void CB_Type::generate_literal(std::ostream& os, void const* raw_data, const Token_context& context, uint32_t depth) const
{
    ASSERT(raw_data); os << *(c_typedef*)raw_data << "UL";
}

void CB_Type::generate_destructor(std::ostream& os, const std::string& id, const Token_context& context, uint32_t depth) const
{

};

void CB_Type::post_circular_reference_error(const Token_context& context) const {
    ASSERT(context != INVALID_CONTEXT, "invalid context should only be used in 100%% safe cases");
    std::ostringstream oss;
    oss << "Circular reference detected in type ";
    generate_type(oss);
    oss << " (" << toS() << ")";
    log_error(oss.str(), context);
}

int CB_Type::get_unique_type_id() {
    static int id=0; // -1 is uninitialized
    ASSERT(id >= 0); // if id is negative then the int has looped around. More than INT_MAX unique types should never be needed.
    return id++;
}


void CB_Type::register_type(const std::string& name, size_t size, void const* default_value)
{
    if (uid == 0 || typenames[uid] != "") {
        uid = get_unique_type_id(); // reuse uid if possible
        // std::cout << "updated uid to " << uid << " for " << name << std::endl;
    }
    ASSERT(typenames[uid] == "", "uid " << uid << " already has name " << typenames[uid] << " colliding with new type " << name); // can't register double value (this should never happen if uid works correctly)
    ASSERT(default_value);
    typenames[uid] = name;
    cb_sizes[uid] = size;
    default_values[uid] = std::move(Any(this, default_value));
}

const Any& CB_Type::default_value() const
{
    const Any& a = default_values[uid];
    ASSERT(a.v_type, "type with uid " << uid << " has no default value!");
    ASSERT(a.v_ptr);

    ASSERT(*a.v_type == *this, "default value type doesn't match: dv.v_type.uid = " << a.v_type->uid << "; *this.uid = " << this->uid);
    return a;
}
