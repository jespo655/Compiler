
// #include "abstx/abstx.h"
// #include "abstx/function.h"
// #include "abstx/identifier.h"
#include "utilities/assert.h"
#include "utilities/unique_id.h"
#include <string>

/*
    TEMPORARY FILE

    The function implementations in this file cannot exist in header files for some reason
    Preferrably, they should be modified so they work in their respective header file

    If that's not possible, they should be moved to other implementation files where they fit in better
*/






// problem: cannot be declared static in a header file - that
// will make separate instances of "id" in different files
uint64_t get_unique_id() {
    static uint64_t id=1;
    ASSERT(id != 0); // if id is 0 then the int has looped around. More than INT_MAX unique identifiers should never be needed.
    return id++;
}

std::string get_unique_id_str() {
    return std::to_string(get_unique_id());
}

void test()
{
    ASSERT(false);
}






// #include "abstx/statements/scope.h"
// #include "abstx/abstx.h"
// // Go up in the Abstx tree until a parent scope is found.
// // If no scope is found, return nullptr
// shared<CB_Scope> Abstx_node::parent_scope() const
// {
//     shared<Abstx_node> abstx = owner;
//     while (abstx != nullptr) {
//         shared<CB_Scope> scope = dynamic_pointer_cast<CB_Scope>(abstx);
//         if (scope != nullptr) return scope;
//         else abstx = abstx->owner;
//     }
//     return nullptr;
// }

// shared<CB_Scope> Abstx_node::global_scope()
// {
//     auto parent = parent_scope();
//     if (parent == nullptr) return dynamic_pointer_cast<CB_Scope>(shared<Abstx_node>(this));
//     if (parent->owner == nullptr) return parent;
//     else return parent->global_scope();
// }





