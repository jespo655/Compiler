#include "abstx_defer.h"

namespace Cube {


std::string Abstx_defer::toS() const override {
    ASSERT(statement != nullptr);
    return "defer " + statement->toS();
}

void Abstx_defer::generate_code(std::ostream& target) const override {
    ASSERT(is_codegen_ready(status));
    statement->generate_code(target);
}

}