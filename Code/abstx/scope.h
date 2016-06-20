#pragma once

#include "type.h"
#include "evaluated_value.h"
#include "statement.h"

struct Literal;
struct Function;
struct Type;
struct Identifier;

#include <map>
#include <vector>
#include <string>
#include <memory>



struct Type_scope : Type {

    bool dynamic = false;

    Type_scope() {}
    Type_scope(bool dynamic) : dynamic{dynamic} {}

    std::string toS() const override { return dynamic? "scope(d)" : "scope(s)"; }
    std::shared_ptr<const Literal> get_default_value() const override;
};



// a scope is its own contained block of code

// scopes can be used two ways: dynamically and statically.
// static scopes can declare identifiers in any order. Everything is processed practically in parallell.
// dynamic scopes perform each action in order. An identifier can only be used after it's declared. (Check that id->context < use->context)

struct Scope : Literal {

    bool dynamic = false;
    std::vector<std::shared_ptr<Statement>> statements;

    // maps from identifier string to abstx node
    std::map<std::string, std::shared_ptr<Identifier>> identifiers;
    std::map<std::string, std::shared_ptr<Function>> functions;
    std::map<std::string, std::shared_ptr<Type>> types; // if looking up the type name in the 'identifier'-list, they would all have the type 'Type_type'. Their values is stored in this list.

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

    std::shared_ptr<Identifier> get_identifier(const std::string& id, bool recursive=true)
    {
        auto p = identifiers[id];
        while (recursive && p == nullptr) {
            auto parent = parent_scope();
            if (parent == nullptr) return nullptr;
            p = parent->get_identifier(id, recursive);
        }
        return p;
    }

    std::shared_ptr<Type> get_type(const std::string& id, bool recursive=true)
    {
        auto p = types[id];
        while (recursive && p == nullptr) {
            auto parent = parent_scope();
            if (parent == nullptr) return nullptr;
            p = parent->get_type(id, recursive);
        }
        return p;
    }

    std::shared_ptr<Function> get_function(const std::string& id, bool recursive=true)
    {
        auto p = functions[id];
        while (recursive && p == nullptr) {
            auto parent = parent_scope();
            if (parent == nullptr) return nullptr;
            p = parent->get_function(id, recursive);
        }
        return p;
    }

    std::shared_ptr<const Type> get_type() override;

};

// Also call Scope Literal_scope, for naming conventions
#define Literal_scope Scope

