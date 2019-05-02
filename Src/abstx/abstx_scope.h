#pragma once

#include "abstx_node.h"
#include "../utilities/flag.h"
#include "statements/abstx_statement.h"
#include "expressions/abstx_identifier.h"

#include <map>

/*
A scope is its own contained block of code

Scopes can be used two ways: dynamically and statically.
Static scopes can declare identifiers in any order. Everything is processed practically in parallell.
Dynamic scopes perform each action in order. An identifier can only be used after it's declared. (Check that id->context < use->context)

Syntax:
{...} // Anonymous scope
Async {...} // Anonymous scope with keywords
Name := {...}; // Named scope
Name := Async {...}; // Named scope with keywords
*/

namespace Cube {

struct Abstx_function_call;
struct Abstx_using;
struct CB_Type;

const flag SCOPE_ASYNC = 1;
const flag SCOPE_DYNAMIC = 2;
const flag SCOPE_SELF_CONTAINED = 3; // should be set if the scope never references identifiers outside itself.

struct Abstx_scope : Abstx_node
{
    Seq<Owned<Statement>> statements;

    std::map<std::string, Shared<Abstx_identifier>> identifiers; // id name -> id. Identifiers are owned by their declaration statements.

    Seq<Shared<Abstx_scope>> imported_scopes;
    Seq<Shared<Abstx_using>> using_statements; // Used in the parsing process. Owned by the list of statements above. Once a using statement has been resolved, it should be returned from this list.
                                                   // FIXME: add a safeguard for when several using-statements tries to import the same scope.
    Seq<Shared<Abstx_function_call>> run_statements;

    uint8_t flags = (uint8_t)SCOPE_SELF_CONTAINED; // scope is self contained until it references something outside its scope
    bool dynamic() const { return flags == SCOPE_DYNAMIC; }
    bool async() const { return flags == SCOPE_ASYNC; }
    bool self_contained() const { return flags == SCOPE_SELF_CONTAINED; }

    Abstx_scope() {}
    Abstx_scope(uint8_t flags) : flags{flags+SCOPE_SELF_CONTAINED} {}
    Abstx_scope(flag flags) : flags{(uint8_t)flags+SCOPE_SELF_CONTAINED} {}

    void debug_print(Debug_os& os, bool recursive=true) const override;
    std::string toS() const override;
    // CB_Object* heap_copy() const override { Abstx_scope* tp = new Abstx_scope(); *tp = *this; return tp; }

    // get the identifier with the given name
    // context is used for potential error messages
    // if the recursive is set, also checks parent scope and imported scopes
    // returns nullpointer if an error occurred. Use add_note() to give additional context
    virtual Shared<Abstx_identifier> get_identifier(const std::string& id, const Token_context& context, bool recursive=true);

    // get the type with the given name
    // context is used for potential error messages
    // if the recursive is set, also checks parent scope and imported scopes
    // returns nullpointer if an error occurred. Use add_note() to give additional context
    virtual Shared<const CB_Type> get_type(const std::string& id, const Token_context& context, bool recursive=true);

    // resolve scopes imported through using statements
    void resolve_imports();

    // generate c code from the scope, outputting it to the target.
    // asserts that the statement is already fully resolved.
    void generate_code(std::ostream& target) const override;

    // fully parse the scope, reading all the statements in it.
    // note that the scope can be fully resolved even if the statements within are not.
    virtual Parsing_status fully_parse();
};



// A global scope corresponds to one compiled file or string. It is always static. It has no owner.
struct Global_scope : Abstx_scope
{
    std::string file_name;
    const Seq<Token> tokens; // should be treated as const
    std::map<uint64_t, Shared<Abstx_function_literal>> used_functions; // map fn_id_uid -> abstx_fn

    Global_scope(Seq<Token>&& tokens) : tokens{std::move(tokens)} {
        add_built_in_types_as_identifiers();
    }

    Token_iterator iterator(int index=0) const { return Token_iterator(tokens, index); }

    Shared<Abstx_scope> parent_scope() const override { return nullptr; }
    Shared<Global_scope> global_scope() const override { return (Global_scope*)this; }

    Parsing_status fully_parse() override;

    Shared<Abstx_function_literal> get_entry_point(const std::string& id);

private:
    static Seq<Owned<Abstx_identifier>> type_identifiers;
    static Token_context built_in_context;

    // add built in types as identifiers in this scope, so they can be accessed with Abstx_scope::get_type()
    void add_built_in_types_as_identifiers();
};



// A function scope is the same as a regular scope, but it can own its own identifiers (from the function signature). It is always dynamic.
struct Abstx_function_scope : Abstx_scope
{
    std::map<std::string, Owned<Abstx_identifier>> fn_identifiers; // id name -> id. Identifiers are owned by the function scope.

    Abstx_function_scope() : Abstx_scope((uint64_t)SCOPE_DYNAMIC) {}

    // function scope specific get_identifier - identifiers can also be a function argument
    // context is used for potential error messages
    // returns nullpointer if an error occurred. Use add_note() to give additional context
    Shared<Abstx_identifier> get_identifier(const std::string& id, const Token_context& context, bool recursive=true) override;

    // @TODO: check if this is necessary. If it is, it should be moved to implementation file
    // void add_identifier(const std::string& name, Shared<const CB_Type> type) {
    //     Owned<Abstx_identifier> id = alloc(Abstx_identifier());
    //     id->name = name;
    //     id->value.v_type = type;
    //     id->owner = this;
    //     fn_identifiers[name] = (std::move(id));
    // }

};



// An anonymous scope is a statement that just consists of a scope literal.
// That begins a new anonymous scope with the current scope as its owner / parent.
// The dynamic-flag is inherited from the current scope.
// Only allowed in a dynamic context (unnamed scopes makes no sense in static context)
struct Abstx_anonymous_scope : Statement
{
    Owned<Abstx_scope> scope = nullptr;

    std::string toS() const override
    {
        ASSERT(scope != nullptr);
        return scope->toS();
    }

    Parsing_status fully_parse() override {
        ASSERT(scope);
        status = scope->fully_parse();
        return status;
    }

    void generate_code(std::ostream& target) const override {
        ASSERT(is_codegen_ready(status));
        scope->generate_code(target);
    };
};

}
