#include "all_abstx.h"


Seq<Owned<Abstx_identifier>> Global_scope::type_identifiers;
Token_context Global_scope::built_in_context;


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

Shared<Abstx_function_literal> Abstx_node::parent_function() const
{
    Shared<Abstx_node> abstx = owner;
    while (abstx != nullptr) {
        Shared<Abstx_function_literal> fn = dynamic_pointer_cast<Abstx_function_literal>(abstx);
        if (fn != nullptr) return fn;
        else abstx = abstx->owner;
    }
    return nullptr;
}



#ifdef DEBUG
Token_iterator Abstx_node::parse_begin(const char* file, int line) const
#else
Token_iterator Abstx_node::parse_begin() const
#endif
{
    ASSERT(status == Parsing_status::PARTIALLY_PARSED, "called from " << file << ":" << line << " with status " << status);
    ASSERT(start_token_index >= 0);
    auto gs = global_scope();
    ASSERT(gs);
    return gs->iterator(start_token_index);
};

// data structure to keep allocated void pointers until the program exits
struct Constant_data_container
{
    std::map<void*,void*> data; // void* mapped to itself

    ~Constant_data_container() {
        for (auto& p : data){
            free(p.second);
            p.second = nullptr;
        }
    }
    void add_constant_data(void* p) {
        data[p] = p;
    }
    void free_constant_data(void* p) {
        free(data[p]);
        data[p] = nullptr;
    }
};

static Constant_data_container _constant_data_container;

void add_constant_data(void* p) { _constant_data_container.add_constant_data(p); }
void* alloc_constant_data(size_t bytes) { _constant_data_container.add_constant_data(malloc(bytes)); }
void free_constant_data(void* p) { _constant_data_container.free_constant_data(p); }
void free_all_constant_data() { _constant_data_container.~Constant_data_container(); }





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
