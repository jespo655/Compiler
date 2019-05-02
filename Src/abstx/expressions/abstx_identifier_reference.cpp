#include "abstx_identifier_reference.h"
#include "abstx_identifier.h"
#include "../abstx_scope.h"

namespace Cube {

void finalize() override {
    if (is_error(status) || is_codegen_ready(status)) return;
    if (id == nullptr) {
        ASSERT(name != ""); // must be set at construction
        id = parent_scope()->get_identifier(name);
    }
    if (id != nullptr) {
        id->finalize();
        status = id->status;
        // LOG("Abstx_identifier_reference.finalize(): finalized identifier " << name << " has status " << id->status);
    } else {
        LOG("Abstx_identifier_reference.finalize(): failed to get identifier " << name << " -> setting Parsing_status::DEPENDENCIES_NEEDED");
        status = Parsing_status::DEPENDENCIES_NEEDED;
    }
}



std::string Abstx_identifier_reference::toS() const override {
    if (id) return id->toS();
    return name;
}

virtual Shared<const CB_Type> Abstx_identifier_reference::get_type() override {
    if (id) return id->get_type();
    else return nullptr;
}

bool Abstx_identifier_reference::has_constant_value() const override {
    ASSERT(id);
    return id->has_constant_value();
}

const Any& Abstx_identifier_reference::get_constant_value() override {
    ASSERT(id);
    return id->get_constant_value();
}

void Abstx_identifier_reference::generate_code(std::ostream& target) const override {
    ASSERT(id);
    return id->generate_code(target);
}


}