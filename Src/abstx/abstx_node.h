#pragma once

#include "../utilities/debug.h"
#include "../utilities/assert.h"
#include "../utilities/pointers.h"
#include "../parser/token.h"
#include "../parser/parsing_status.h"
#include "../parser/token_iterator.h"

#include <string>
#include <ostream>
#include <iostream> // for debug purposes

struct Abstx_function_literal;
struct Abstx_scope;
struct Global_scope;

/*
    An Abstx_node is a node in the abstract syntax tree.
    It has a pointer to its owner (using polymorphism)
    It knows its own start token index (so it can create a new iterator and index through the tokens later)
        and token context (so it can give correct error messages).
*/
struct Abstx_node : public Serializable
{
    // should be set immediately on creation:
    Shared<Abstx_node> owner = nullptr; // points to the parent node in the abstx tree
    Token_context context;
    int start_token_index = -1; // Points to the first token in the expression

    // should be set a bit later:
    Parsing_status status = Parsing_status::NOT_PARSED;

    // generate_code(): generate valid c code and output it to to target.
    // This is done recursively to ensure that all dependencies are outputted first.
    virtual void generate_code(std::ostream& target, const Token_context& context) const = 0;

    // Set owner in a type safe way
    template<typename T> void set_owner(const T& p)
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

    // Return a pointer to the closest parent scope or function in the tree
    virtual Shared<Abstx_scope> parent_scope() const;
    virtual Shared<Abstx_function_literal> parent_function() const;

    // Return a pointer to the root parent scope in the tree
    virtual Shared<Global_scope> global_scope() const;

    // make basic assertions and get the correct token iterator; to be called in the beginning of fully_parse()
    #ifdef DEBUG
    Token_iterator parse_begin(const char* file = __builtin_FILE(), int line = __builtin_LINE()) const; // is supposed to be called with no arguments
    #else
    Token_iterator parse_begin() const;
    #endif

};

// functions to keep allocated void* safe until program terminates
// when the program terminates or when free_all_constant_data() is called, all given void* is freed with free()
void add_constant_data(void* p);
void* alloc_constant_data(size_t bytes);
void free_constant_data(void* p);
void free_all_constant_data();
