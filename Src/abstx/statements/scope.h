#pragma once

#include "statement.h"
#include "using.h"
#include "../expressions/identifier.h"

#include <map>

/*
A scope is its own contained block of code

Scopes can be used two ways: dynamically and statically.
Static scopes can declare identifiers in any order. Everything is processed practically in parallell.
Dynamic scopes perform each action in order. An identifier can only be used after it's declared. (Check that id->context < use->context)

CB_Scope is actually a CB class, but since it has dependencies on abstx, it is defined here instead,
avoiding cross dependencies between packages.

Syntax:
{...} // Anonymous scope
Async {...} // Anonymous scope with keywords
Name := {...}; // Named scope
Name := Async {...}; // Named scope with keywords
*/

const CB_Flag SCOPE_ASYNC = 1;
const CB_Flag SCOPE_DYNAMIC = 2;
const CB_Flag SCOPE_SELF_CONTAINED = 3; // should be set if the scope never references identifiers outside itself.

struct CB_Scope : Abstx_node //, CB_Object
{
    // static CB_Type type;
    seq<owned<Statement>> statements;

    std::map<CB_String, shared<Identifier>> identifiers; // id name -> id. Identifiers are owned by their declaration statements.

    seq<shared<CB_Scope>> imported_scopes;
    seq<shared<Using_statement>> using_statements; // Used in the parsing process. Owned by the list of statements above. Once a using statement has been resolved, it should be returned from this list.
                                                   // FIXME: add a safeguard for when several using-statements tries to import the same scope.
    CB_u8 flags = 0;
    bool dynamic() const { return flags == SCOPE_DYNAMIC; }
    bool async() const { return flags == SCOPE_ASYNC; }
    bool self_contained() const { return flags == SCOPE_SELF_CONTAINED; }

    CB_Scope() {}
    CB_Scope(CB_u8 flags) : flags{flags} {}

    void debug_print(Debug_os& os, bool recursive=true) const override {
        os << "{ // " << toS() << std::endl;
        os.indent();
        for (auto& s : statements) {
            ASSERT(s != nullptr);
            s->debug_print(os, recursive);
        }
        os.unindent();
        os << "}" << std::endl;
    }
    std::string toS() const override { return dynamic()? "scope(d)" : "scope(s)"; }
    // CB_Object* heap_copy() const override { CB_Scope* tp = new CB_Scope(); *tp = *this; return tp; }

    shared<Identifier> get_identifier(const CB_String& id, bool recursive=true)
    {
        auto p = identifiers[id];
        ASSERT(!self_contained() || p != nullptr)
        if (p != nullptr) return p; // local things goes first

        if (recursive) {
            // check parent scope
            while (p == nullptr) {
                auto parent = parent_scope();
                if (parent == nullptr) break;
                p = parent->get_identifier(id, recursive);
            }

            // check imported scopes
            if (using_statements.size > 0) resolve_imports();
            for (auto scope : imported_scopes) {
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

    // shared<CB_Scope> get_scope(const CB_String& id, bool recursive=true)
    // {
    //     auto p = get_identifier(id, recursive);
    //     if (p == nullptr || *(p->type) != CB_Scope::type) return nullptr;
    //     return &(p->value.value<CB_Scope>());
    //     return nullptr;
    // }

    void resolve_imports()
    {
        seq<shared<Using_statement>> remaining;

        while (using_statements.size > 0) {
            remaining.clear();

            for (auto us : using_statements) {
                if (us->status == Parsing_status::NOT_PARSED || us->status == Parsing_status::PARTIALLY_PARSED) {
                    us->status = Parsing_status::DEPENDENCIES_NEEDED;

                    Parsing_status status; // = fully_resolve(us->subject); // FIXME: fully_resolve(value_expr)
                    if (status == Parsing_status::FULLY_RESOLVED) {
                        ASSERT(false, "FIXME: eval");
                        CB_Any scope;// = eval(us->subject); // FIXME: eval(value_expr)
                        if (scope.type == CB_String::type) {
                            ASSERT(false, "FIXME: string import");
                            // compile file, import its global scope
                        // } else if (scope.type == CB_Scope::type) {
                        //     // import the scope directly
                        //     ASSERT(false, "FIXME: scope import");
                        //     // imported_scopes.add(scope.value<CB_Scope>());
                        } else {
                            log_error("Invalid type in using statement: expected string or scope, but found "+scope.type.toS(), us->context);
                            // if nullptr, eval should already have logged error
                        }
                    } else if (status == Parsing_status::DEPENDENCIES_NEEDED) {
                        remaining.add(us);
                    } else {
                        ASSERT(is_error(status)); // ignore the error
                    }
                }
            }

            if (using_statements.size == remaining.size) {
                for (auto& us : remaining) {
                    log_error("Unable to resolve using statement", us->context);
                }
                using_statements.clear();
                break;
            }

            using_statements = std::move(remaining);
        }
        ASSERT(using_statements.size == 0);
    }

};




// An anonymous scope is a statement that just consists of a scope literal.
// That begins a new anonymous scope with the current scope as its owner / parent.
// The dynamic-flag is inherited from the current scope.

struct Anonymous_scope : Statement
{
    owned<CB_Scope> scope;

    std::string toS() const override
    {
        ASSERT(scope != nullptr);
        return scope->toS();
    }
};












