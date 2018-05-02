#include "all_abstx.h"

#include "statements/abstx_scope.h"
#include "abstx.h"

// Go up in the Abstx tree until a parent scope is found.
// If no scope is found, return nullptr
Shared<Abstx_scope> Abstx_node::parent_scope() const
{
    Shared<Abstx_node> abstx = owner;
    while (abstx != nullptr) {
        Shared<Abstx_scope> scope = dynamic_pointer_cast<Abstx_scope>(abstx);
        if (scope != nullptr) return scope;
        else abstx = abstx->owner;
    }
    return nullptr;
}

Shared<Abstx_scope> Abstx_node::global_scope()
{
    auto parent = parent_scope();
    if (parent == nullptr) return dynamic_pointer_cast<Abstx_scope>(Shared<Abstx_node>(this));
    if (parent->owner == nullptr) return parent;
    else return parent->global_scope();
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
    { Abstx_function abstx; }
    { Abstx_function_call abstx; }
    // { Abstx_if abstx; } // undefined reference to bool
    { Abstx_return abstx; }
    // { Abstx_scope abstx; } // log_error undefined
    // { Statement abstx; } // abstract class
    { Abstx_using abstx; }
    // { Abstx_while abstx; } // undefined reference to bool
}

#endif
