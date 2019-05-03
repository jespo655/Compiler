#include "abstx_defer.h"


std::string Abstx_defer::toS() const {
    ASSERT(statement != nullptr);
    return "defer " + statement->toS();
}

void Abstx_defer::generate_code(std::ostream& target) const {
    ASSERT(is_codegen_ready(status));
    statement->generate_code(target);
}
