#pragma once

#include "../utilities/debug.h"
#include "../utilities/assert.h"
#include "../token.h"

#include <memory>
#include <iostream>

struct Scope;

#include <string>

struct Abstx_node
{
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

    std::shared_ptr<const Token_context> context = nullptr; // points to the beginning of the code
    std::weak_ptr<Abstx_node> owner; // points to the parent node in the abstx tree
    bool fully_resolved = false; // should be true when all children are resolved

    virtual ~Abstx_node() {}

    virtual std::shared_ptr<Scope> parent_scope() const;
    virtual std::shared_ptr<Scope> global_scope() const;
};



