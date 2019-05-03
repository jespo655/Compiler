#include "abstx_identifier.h"
#include "../statements/abstx_statement.h"
#include "../../utilities/unique_id.h"

using namespace Cube;

std::string Abstx_identifier::toS() const {
    ASSERT(name.length() > 0);
    std::ostringstream oss;
    oss << name << ":";
    if (value.v_type == nullptr) oss << "???";
    else oss << value.v_type->toS();
    if (value.v_ptr != nullptr) oss << "=" << value.toS();
    return oss.str();
}

Shared<const CB_Type> Abstx_identifier::get_type() {
    if (value.v_type == nullptr && value_expression != nullptr) value.v_type == value_expression->get_type();
    return value.v_type;
}

Shared<const CB_Type> Abstx_identifier::get_constant_type() const {
    return value.v_type;
}

bool Abstx_identifier::has_constant_value() const {
    return value.v_ptr != nullptr || (value_expression != nullptr && value_expression->has_constant_value());
}

const Any& Abstx_identifier::get_constant_value() {
    if (value.v_ptr != nullptr) return value;
    if (value_expression != nullptr && value_expression->has_constant_value()) value.v_ptr = value_expression->get_constant_value().v_ptr;
    return value;
}

void Abstx_identifier::generate_code(std::ostream& target) const
{
    // this should ouput the identifier used as a variable, since it's a subclass of Variable_expression
    ASSERT(name != "");
    ASSERT(is_codegen_ready(status), "id: "+name);
    target << name;
    if (uid) target << "_" << uid; // uid suffix to avoid name C name clashes
}

void Abstx_identifier::finalize() {
    if (is_codegen_ready(status)) return;
    ASSERT(name != ""); // must be set during creation
    if (value.v_type == nullptr) {
        status = Parsing_status::DEPENDENCIES_NEEDED; // set this for now to avoid cyclic dependencies
        // try to resolve the owning declaration statement
        Shared<Statement> decl = dynamic_pointer_cast<Statement>(owner);
        if (decl != nullptr) {
            decl->fully_parse();
        } else {
            // id could be owned by a struct or function
            ASSERT(is_error(status)); // if not error, we should have been able to infer type by now
        }
    }
    if (value.v_type == nullptr) {
        ASSERT(is_error(status) || status == Parsing_status::DEPENDENCIES_NEEDED);
        return; // still not resolved -> probably failed to resolve declaration, just return
    } else {
        status = Parsing_status::FULLY_RESOLVED;
    }
}
