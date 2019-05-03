#include "abstx_node.h"
#include "expressions/abstx_function.h"
#include "abstx_scope.h"

using namespace Cube;


void Abstx_node::debug_print(Debug_os& os, bool recursive) const
{
    os << toS() << std::endl;
}




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

Shared<Global_scope> Abstx_node::global_scope() const
{
    return owner->global_scope();
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








// @TODO: move this to a separate file?

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

