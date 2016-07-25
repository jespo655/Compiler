#pragma once

#include "type.h"
#include "statement.h"
#include "literal.h"

struct Function;
struct Type;
struct Identifier;
struct Using_statement;

#include <map>
#include <vector>
#include <string>
#include <memory>



// a scope is its own contained block of code

// scopes can be used two ways: dynamically and statically.
// static scopes can declare identifiers in any order. Everything is processed practically in parallell.
// dynamic scopes perform each action in order. An identifier can only be used after it's declared. (Check that id->context < use->context)

struct Scope : Literal {

    bool dynamic = false; // FIXME: maybe not needed. remove? Dynamic-ness can be inferred from the context instead. (no dynamic scopes are partially parsed in parsing pass 1)
    std::vector<std::shared_ptr<Statement>> statements;

    // maps from identifier string to abstx node
    std::map<std::string, std::shared_ptr<Identifier>> identifiers;
    std::map<std::string, std::shared_ptr<Function>> functions;
    std::map<std::string, std::shared_ptr<Type>> types; // if looking up the type name in the 'identifier'-list, they would all have the type 'Type_type'. Their values is stored in this list.

    std::vector<std::shared_ptr<Scope>> pulled_in_scopes; // FIXME: find a way to not import the same scope more than once
    std::vector<std::shared_ptr<Using_statement>> using_statements; // used in the parsing process

    Scope() {}
    Scope(bool dynamic) : dynamic{dynamic} {}

    void debug_print(Debug_os& os, bool recursive=true) const override {
        os << "{ // " << toS() << std::endl;
        os.indent();
        for (auto s : statements) {
            ASSERT(s != nullptr);
            s->debug_print(os, recursive);
        }
        os.unindent();
        os << "}" << std::endl;
    }

    std::string toS() const override { return dynamic? "scope(d)" : "scope(s)"; }

    // Getter functions for identifiers, types, and functions
    // If recursive=true, also checks through parent scopes

    // FIXME: these should probably be const

    virtual std::shared_ptr<Identifier> get_identifier(const std::string& id, bool recursive=true)
    {
        auto p = identifiers[id];
        if (p != nullptr) return p; // local things goes first

        if (recursive) {
            // check parent scope
            while (p == nullptr) {
                auto parent = parent_scope();
                if (parent == nullptr) break;
                p = parent->get_identifier(id, recursive);
            }

            // check pulled in scopes
            for (auto scope : pulled_in_scopes) {
                // auto scope = scope_pair.second;
                auto p2 = scope->get_identifier(id, false);
                if (p == nullptr) p = p2;
                else {
                    // FIXME: log error identifier clash
                    // first found here: p->context()
                }
            }
        }
        return p;
    }

    virtual std::shared_ptr<Type> get_type(const std::string& id, bool recursive=true)
    {
        auto p = types[id];
        if (p != nullptr) return p; // local things goes first

        if (recursive) {
            // check parent scope
            while (p == nullptr) {
                auto parent = parent_scope();
                if (parent == nullptr) break;
                p = parent->get_type(id, recursive);
            }

            // check pulled in scopes
            for (auto scope : pulled_in_scopes) {
                // auto scope = scope_pair.second;
                auto p2 = scope->get_type(id, false);
                if (p == nullptr) p = p2;
                else {
                    // FIXME: log error identifier clash
                    // first found here: p->context()
                }
            }
        }
        return p;
    }

    virtual std::shared_ptr<Function> get_function(const std::string& id, bool recursive=true)
    {
        auto p = functions[id];
        if (p != nullptr) return p; // local things goes first

        if (recursive) {
            // check parent scope
            while (p == nullptr) {
                auto parent = parent_scope();
                if (parent == nullptr) break;
                p = parent->get_function(id, recursive);
            }

            // check pulled in scopes
            for (auto scope : pulled_in_scopes) {
                // auto scope = scope_pair.second;
                auto p2 = scope->get_function(id, false);
                if (p == nullptr) p = p2;
                else {
                    // FIXME: log error identifier clash
                    // first found here: p->context()
                }
            }
        }
        return p;
    }

};

// Also call Scope Literal_scope, for naming conventions
#define Literal_scope Scope



/*
An anonymous scope is a statement that just consists of a scope literal.
That begins a new anonymous scope with the current scope as its owner / parent.
The dynamic-flag is inherited from the current scope.
*/
struct Anonymous_scope : Statement
{
    std::shared_ptr<Literal_scope> scope;

    std::string toS() const override
    {
        ASSERT(scope != nullptr);
        return scope->toS();
    }
};






struct Type_scope : Type {

    bool dynamic = false;

    Type_scope() {}
    Type_scope(bool dynamic) : dynamic{dynamic} {}


    static std::shared_ptr<Scope> cpp_value(void const* value_ptr, int size=0) {
        ASSERT(size == 0 || size == sizeof(std::shared_ptr<Scope>));
        std::shared_ptr<Scope>* const ptr_ptr = (std::shared_ptr<Scope>* const)value_ptr;
        return *ptr_ptr;
    }

    std::string toS(void const * value_ptr, int size=0) const override {
        return cpp_value(value_ptr, size)->toS();
    }


    std::string toS() const override { return dynamic? "scope(d)" : "scope(s)"; }

    int byte_size() const override { return sizeof(std::shared_ptr<Scope>); } // points to the scope struct
};

