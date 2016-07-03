
#include "abstx/abstx.h"
#include "abstx/function.h"
#include "abstx/identifier.h"
#include "utilities/assert.h"
#include "utilities/unique_id.h"
#include <string>

/*
    TEMPORARY FILE

    The function implementations in this file cannot exist in header files for some reason
    Preferrably, they should be modified so they work in their respective header file

    If that's not possible, they should be moved to other implementation files where they fit in better
*/


// problem: cannot dynamic_cast to pointer to incomplete type
// Scope is incomplete until Abstx_node is complete

// Go up in the Abstx tree until a parent scope is found.
// If no scope is found, return nullptr
std::shared_ptr<Scope> Abstx_node::parent_scope() const
{
    std::shared_ptr<Abstx_node> abstx = owner.lock();
    while (abstx != nullptr) {
        if (std::shared_ptr<Scope> scope = std::dynamic_pointer_cast<Scope>(abstx)) {
            return scope;
        } else {
            abstx = abstx->owner.lock();
        }
    }
    return nullptr;
}


// global scope is probably not needed, so it's commented out for now
std::shared_ptr<Scope> Abstx_node::global_scope() const
{
    auto parent = parent_scope();
    if (parent == nullptr) return nullptr;
    if (parent->owner.lock() == nullptr) return parent;
    else return parent->global_scope();
}










// problem: cannot be declared static in a header file - that
// will make separate instances of "id" in different files
int get_unique_id() {
    static int id=0;
    ASSERT(id >= 0); // if id is negative then the int has looped around. More than INT_MAX unique identifiers should never be needed.
    return id++;
}

std::string get_unique_id_str() {
    return std::to_string(get_unique_id());
}










