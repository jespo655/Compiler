#include "abstx_identifier_reference.h"
#include "abstx_identifier.h"
#include "../abstx_scope.h"

using namespace Cube;

void Abstx_identifier_reference::finalize() {
    if (is_error(status) || is_codegen_ready(status)) return;
    if (id == nullptr) {
        ASSERT(name != ""); // must be set at construction
        id = parent_scope()->get_identifier(name, context);
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



std::string Abstx_identifier_reference::toS() const {
    if (id) return id->toS();
    return name;
}

Shared<const CB_Type> Abstx_identifier_reference::get_type() {
    if (id) return id->get_type();
    else return nullptr;
}

bool Abstx_identifier_reference::has_constant_value() const {
    ASSERT(id);
    return id->has_constant_value();
}

const Any& Abstx_identifier_reference::get_constant_value() {
    ASSERT(id);
    return id->get_constant_value();
}

void Abstx_identifier_reference::generate_code(std::ostream& target) const {
    ASSERT(id);
    return id->generate_code(target);
}


