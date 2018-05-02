#pragma once

#include "../utilities/debug.h"
#include "../utilities/assert.h"
#include "../utilities/pointers.h"
#include "../parser/token.h"
#include "../parser/parsing_status.h"

#include <string>
#include <ostream>
#include <iostream> // for debug purposes

struct Abstx_scope;

/*
    An Abstx_node is a node in the abstract syntax tree.
    It has a pointer to its owner (using polymorphism)
    It knows its own start token index (so it can create a new iterator and index through the tokens later)
        and token context (so it can give correct error messages).
*/
struct Abstx_node
{
    // should be set immediately on creation:
    Shared<Abstx_node> owner = nullptr; // points to the parent node in the abstx tree
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

    // finalize(): check that all dependencies are finalized and that there are no errors.
    // This is done recursively. The parsing status is updated, then returned.
    // This function should only be called when the abstx node has finished parsing, and is expected to be complete.
    virtual Parsing_status finalize() = 0;

    // generate_code(): generate code and output it to to target.
    // This is done recursively to ensure that all dependencies are outputted first.
    virtual void generate_code(std::ostream& target) {
        target << "/* " << toS() << " */";
    };

    // Set owner in a type safe way
    template<typename T> void set_owner(const Shared<T>& p)
    {
        auto abstx_p = dynamic_pointer_cast<Abstx_node>(p);
        ASSERT(abstx_p != nullptr); // this checks for both if p is nullptr, and if T is a subclass of Abstx_node
        owner = abstx_p;
    }

    // default constructor
    Abstx_node() {}

    // constructor that automatically sets owner
    Abstx_node(Shared<Abstx_node> owner) : owner{owner} {}

    // Virtual destructor needed for polymorphism
    virtual ~Abstx_node() {}

    // Return a pointer to the closest parent scope in the tree
    virtual Shared<Abstx_scope> parent_scope() const;

    // Return a pointer to the root parent scope in the tree
    virtual Shared<Abstx_scope> global_scope();
};



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
    Literal (bool, int, string, float, Seq, map)
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
