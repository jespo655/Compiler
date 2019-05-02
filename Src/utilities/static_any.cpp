#include "static_any.h"


namespace Cube {


static std::string toS_void_callback(const Static_any& any)
{
    ASSERT(any.v_ptr == nullptr);
    return "any(void)";
}

static void destroy_void_callback(Static_any& any)
{
    ASSERT(any.v_ptr == nullptr);
}

static void assign_void_callback(Static_any& obj, const Static_any& any)
{
    ASSERT(any.v_ptr == nullptr);
    obj.~Static_any();
}



Static_any& Static_any::operator=(const Static_any& any) {
    // std::cout << "any copy op = any of type " << any.v_type.toS() << std::endl;
    any.assign_callback(*this, any);
}

Static_any& Static_any::operator=(Static_any&& any) {
    // std::cout << "any move op = any of type " << any.v_type.toS() << std::endl;
    v_type = any.v_type;
    v_ptr = any.v_ptr;
    destructor_callback = any.destructor_callback;
    toS_callback = any.toS_callback;
    assign_callback = any.assign_callback;
    any.v_ptr = nullptr;
    /*std::cout << "move cstr (on old any) ";*/ any.set_default_callbacks();
}

void Static_any::set_default_callbacks() {
    // std::cout << "setting default callbacks" << std::endl;
    destructor_callback = destroy_void_callback;
    toS_callback = toS_void_callback;
    assign_callback = assign_void_callback;
}

}
