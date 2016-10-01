#pragma once

#include "../utilities/debug.h"
#include "../utilities/assert.h"
#include "../parser/token.h"
#include "../parser/parsing_status.h"

#include <string>
#include <iostream> // for debug purposes

#include "../types/cb_types.h"
template<typename T> using shared = CB_Sharing_pointer<T>;
template<typename T> using owned = CB_Owning_pointer<T>;
template<typename T> using seq = CB_Dynamic_seq<T>;
template<typename T, int i> using fixed_seq = CB_Static_seq<T,i>;

struct CB_Scope;

/*
    An Abstx_node is a node in the abstract syntax tree.
    It has a pointer to its owner (using polymorphism)
    It knows its own start token index (so it can create a new iterator and index through the tokens later)
        and token context (so it can give correct error messages).
*/
struct Abstx_node
{
    // should be set immediately on creation:
    shared<Abstx_node> owner; // points to the parent node in the abstx tree
    Token_context context;
    int start_token_index = -1; // Points to the first token in the expression

    // should be set a bit later:
    Parsing_status status = Parsing_status::NOT_PARSED;

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

    // Set owner in a type safe way
    template<typename T> void set_owner(const shared<T>& p)
    {
        auto abstx_p = dynamic_pointer_cast<Abstx_node>(p);
        ASSERT(abstx_p != nullptr); // this checks for both if p is nullptr, and if T is a subclass of Abstx_node
        owner = abstx_p;
    }

    // Virtual destructor needd for polymorphism
    virtual ~Abstx_node() {}

    // Return a pointer to the closest parent scope in the tree
    virtual shared<CB_Scope> parent_scope() const;

    // Return a pointer to the root parent scope in the tree
    virtual shared<CB_Scope> global_scope() ;
};

#include "statements/scope.h"
// Go up in the Abstx tree until a parent scope is found.
// If no scope is found, return nullptr
shared<CB_Scope> Abstx_node::parent_scope() const
{
    shared<Abstx_node> abstx = owner;
    while (abstx != nullptr) {
        shared<CB_Scope> scope = dynamic_pointer_cast<CB_Scope>(abstx);
        if (scope != nullptr) return scope;
        else abstx = abstx->owner;
    }
    return nullptr;
}

shared<CB_Scope> Abstx_node::global_scope()
{
    auto parent = parent_scope();
    if (parent == nullptr) return dynamic_pointer_cast<CB_Scope>(shared<Abstx_node>(this));
    if (parent->owner == nullptr) return parent;
    else return parent->global_scope();
}



/*


Statement:
    If
    For
    While
    Return
    Assignment
    Declaration
    Using
    Scope declaration (Anonymous or Named, maybe with keyword modifiers such as Async)
    Pure value_expression (operators or function call with side effects)
    Defer
Modifiers: Generic

Value_expression:
    Variable_expression
    Literal (bool, int, string, float, seq, map)
    Function call

Variable_expression:
    Identifier
    Getter




deprecated:
    // Cast (?)            // equivalent with function call
    // Prefix operator     // equivalent with function call
    // Infix operator      // equivalent with function call
    // Array indexing      // equivalent with function call that returns a sharing pointer





// TODO: scope syntax

// Anonymous scope:
{...}

// Named scope:
Name {...}
Name :: {...}; // maybe?


// Keywords:
Async {...}
Async Name {...}














*/