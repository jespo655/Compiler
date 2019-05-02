

Seq<Owned<Abstx_identifier>> Global_scope::type_identifiers;
Token_context Global_scope::built_in_context;


Shared<Abstx_function_literal> Global_scope::get_entry_point(const std::string& id) {
    auto fn_id = get_identifier(id);
    Shared<const CB_Type> t = fn_id->get_type();
    ASSERT(t != nullptr); // type must be known
    Shared<const CB_Function> fn_type = dynamic_pointer_cast<const CB_Function>(t);
    if (fn_type == nullptr) {
        log_error("No entry point defined!", context);
        return nullptr;
    } else if (fn_id->has_constant_value()) {
        return (Abstx_function_literal*)fn_id->get_constant_value().v_ptr;
    }
}



#ifdef TEST

// not very useful test location since almost all things we would want to test here requires other cpp files (e.g. types)

#include "../types/all_cb_types.h"
#include <iostream>

int main()
{
    { Abstx_assignment abstx; }
    { Abstx_declaration abstx; }
    { Abstx_defer abstx; }
    // { Abstx_for abstx; }
    { Abstx_function_literal abstx; }
    { Abstx_function_call abstx; }
    // { Abstx_if abstx; } // undefined reference to bool
    { Abstx_return abstx; }
    // { Abstx_scope abstx; } // log_error undefined
    // { Statement abstx; } // abstract class
    { Abstx_using abstx; }
    // { Abstx_while abstx; } // undefined reference to bool
}

#endif
