#include "abstx_pointer_deferencerence.h"
#include "abstx_identifier.h"
// #include "../../types/cb_pointer.h" // @TODO: check if this is necessary

namespace Cube {

std::string Abstx_pointer_dereference::toS() const override {
    ASSERT(pointer_id->name.length() > 0);
    std::ostringstream oss;
    oss << "*" << pointer_id->name;
    return oss.str();
}

virtual Shared<const CB_Type> Abstx_pointer_dereference::get_type() override {
    ASSERT(pointer_id);
    Shared<const CB_Pointer> type = dynamic_pointer_cast<const CB_Pointer>(pointer_id->get_type());
    ASSERT(type != nullptr);
    ASSERT(type->v_type != nullptr);
    return type->v_type;
}

void Abstx_pointer_dereference::generate_code(std::ostream& target) const override
{
    ASSERT(is_codegen_ready(status));
    target << "*";
    pointer_id->generate_code(target);
}




std::string Abstx_address_of::toS() const override {
    ASSERT(pointer_id->name.length() > 0);
    std::ostringstream oss;
    oss << "*" << pointer_id->name;
    return oss.str();
}

virtual Shared<const CB_Type> Abstx_address_of::get_type() override {
    ASSERT(pointer_id);
    if (pointer_type == nullptr) {
        Owned<CB_Pointer> pt = alloc(CB_Pointer());
        pt->v_type = pointer_id->get_type();
        pt->finalize();
        pointer_type = add_complex_cb_type(owned_static_cast<CB_Type>(std::move(pt)));
    }
    return pointer_type;
}

void Abstx_address_of::generate_code(std::ostream& target) const override
{
    ASSERT(is_codegen_ready(status));
    target << "&";
    pointer_id->generate_code(target);
}

}
