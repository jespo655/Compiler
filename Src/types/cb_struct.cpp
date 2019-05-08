#include "cb_struct.h"

#include <iomanip>


std::string CB_Struct::Struct_member::toS() const {
    std::ostringstream oss;
    oss << (is_using?"using ":"")
        << id->name << ":"
        << type->toS() << "="
        << (explicit_uninitialized?"---":id->value.toS());
    return oss.str();
}



CB_Struct& CB_Struct::operator=(const CB_Struct& sm) {
    if (this != &sm) {
        uid=sm.uid; members = sm.members; max_alignment=sm.max_alignment;
        free(_default_value);
        _default_value = malloc(sm.cb_sizeof());
        memcpy(_default_value, sm._default_value, sm.cb_sizeof());
    }
    return *this;
}
CB_Struct& CB_Struct::operator=(CB_Struct&& sm) {
    if (this != &sm) {
        uid=sm.uid;
        members = std::move(sm.members);
        _default_value = sm._default_value; sm._default_value = nullptr;
        max_alignment=sm.max_alignment;
    }
    return *this;
}

CB_Struct::~CB_Struct() { free(_default_value); }



std::string CB_Struct::toS() const {
    std::ostringstream oss;
    oss << "struct { ";
    for (int i = 0; i < members.size; ++i) {
        oss << members[i].toS();
        oss << "; ";
    }
    oss << "}";
    return oss.str();
}

void CB_Struct::add_member(const Shared<Abstx_identifier>& id, bool is_using) {
    ASSERT(id != nullptr);
    members.add(Struct_member(id, is_using));
}

Shared<const CB_Struct::Struct_member> CB_Struct::get_member(const std::string& id) const {
    for (auto& member : members) {
        if (member.id->name == id) return &member;
    }
    // @todo: check members marked using recursively
    return nullptr;
}



void CB_Struct::finalize() {
    std::cout << "finalizing struct type" << std::endl;
    size_t total_size = 0;
    if (members.empty()) {
        total_size = 1;
        _default_value = malloc(total_size);
        // actual default value doesn't matter since it will never be used anyway
    } else {
        // go through all members, assign them byte positions
        // (TODO: rearrange the members if it would save space)
        for (auto& member : members) {
            // add memory alignment for 16 / 32 bit or bigger values (since this is done in C by default)
            size_t alignment = member.id->value.v_type->alignment();
            align(&total_size, alignment);
            member.byte_position = total_size;
            total_size += member.id->value.v_type->cb_sizeof();
            if (alignment > max_alignment) max_alignment = alignment;
        }

        // fix final alignment so it works in sequences
        align(&total_size, max_alignment);

        // copy default value
        _default_value = malloc(total_size);
        for (auto& member : members) {
            ASSERT(member.id->value.v_type != nullptr);
            if (member.id->value.v_ptr == nullptr) member.id->value = member.id->value.v_type->default_value();
            memcpy((uint8_t*)_default_value+member.byte_position, member.id->value.v_ptr, member.id->value.v_type->cb_sizeof());
        }
    }
    std::cout << "finishging assertions and register_type()" << std::endl;
    ASSERT(total_size > 0);
    ASSERT(_default_value != nullptr);
    register_type(toS(), total_size, _default_value);
}


// code generation functions
void CB_Struct::generate_typedef(std::ostream& os) const {
    ASSERT(_default_value); // assert finalized
    os << "typedef struct{";
    if (ONELINE_STRUCT_DEFINITIONS && members.size>0) os << " ";
    for (const auto& member : members) {
        if (!ONELINE_STRUCT_DEFINITIONS) os << std::endl;
        member.id->value.v_type->generate_type(os);
        os << " ";
        member.id->generate_code(os);
        os << "; ";
    }
    if (!ONELINE_STRUCT_DEFINITIONS && members.size>0) os << std::endl;
    os << "} ";
    generate_type(os);
    os << ";" << std::endl;
}

void CB_Struct::generate_literal(std::ostream& os, void const* raw_data, uint32_t depth) const {
    ASSERT(_default_value, "struct must be finalized before it can be used"); // assert finalized
    ASSERT(raw_data != nullptr);
    if (depth > MAX_ALLOWED_DEPTH) { post_circular_reference_error(); os << "void"; return; }
    os << "(";
    generate_type(os);
    os << "){";
    for (int i = 0; i < members.size; ++i) {
        if (i) os << ", ";
        members[i].id->value.v_type->generate_literal(os, (uint8_t const*)raw_data+members[i].byte_position);
    }
    os << "}";
}

void CB_Struct::generate_destructor(std::ostream& os, const std::string& id, uint32_t depth) const {
    if (depth > MAX_ALLOWED_DEPTH) { post_circular_reference_error(); return; }
    for (const auto& member : members) {
        member.id->value.v_type->generate_destructor(os, id + "." + member.id->name, depth+1);
    }
}