#pragma once

#include "../utilities/debug.h"
#include "../utilities/assert.h"
#include "../parser/token.h"
#include "../parser/parsing_status.h"

#include <memory>
#include <iostream>
#include <string>

struct Scope;

struct Abstx_node
{
    std::weak_ptr<Abstx_node> owner; // points to the parent node in the abstx tree
    Token_context context;

    Parsing_status status = Parsing_status::NOT_PARSED;
    int start_token_index; // Points to the first token in the expression

    // debug_print(): print all data about the node, on one or several lines.
    // If recursive=true, then also call debug_print() on all child nodes.
    // For child nodes, use os.indent() and os.unindent() for clarity.
    // As default debug_print() prints toS().
    virtual void debug_print(Debug_os& os, bool recursive=true) const { os << toS() << std::endl; }

    // toS(): concatenate the most basic data about the node in a single
    // line of text. This line should preferrably be self contained and fit
    // well into other text.
    // This method should always be implemented in all non-abstract classes.
    virtual std::string toS() const = 0;

    // check if the syntax node is fully resolved
    virtual bool fully_resolved() const { return status == Parsing_status::FULLY_RESOLVED; }

    // try_resolve() should try to resolve the abstx as much as possible and update the parsing_status with the new status.
    // In some cases where the status is not expected to change (for example with syntax errors), the status can be returned immediately.
    virtual Parsing_status try_resolve() { return status; };

    // Set owner in a type safe way
    template<typename T> void set_owner(std::shared_ptr<T> p)
    {
        auto abstx_p = std::dynamic_pointer_cast<Abstx_node>(p);
        ASSERT(abstx_p != nullptr); // this checks for both if p is nullptr, and if T is a subclass of Abstx_node
        owner = abstx_p;
    }

    virtual ~Abstx_node() {}

    virtual std::shared_ptr<Scope> parent_scope() const;
    // virtual std::shared_ptr<Scope> global_scope() const;
};



