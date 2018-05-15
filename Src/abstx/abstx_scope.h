#pragma once

#include "statements/abstx_statement.h"
#include "statements/abstx_using.h"
#include "expressions/abstx_identifier.h"
#include "../utilities/flag.h"
#include "../types/cb_string.h"

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

const flag SCOPE_ASYNC = 1;
const flag SCOPE_DYNAMIC = 2;
const flag SCOPE_SELF_CONTAINED = 3; // should be set if the scope never references identifiers outside itself.

struct Abstx_function_call;

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
    Abstx_scope(uint8_t flags) : flags{flags} {}

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
    // CB_Object* heap_copy() const override { Abstx_scope* tp = new Abstx_scope(); *tp = *this; return tp; }

    virtual Shared<Abstx_identifier> get_identifier(const std::string& id, bool recursive=true)
    {
        auto p = identifiers[id];
        if (p != nullptr) return p; // local things goes first

        if (recursive) {
            // check parent scope
            auto parent = parent_scope();
            if (parent != nullptr) {
                p = parent->get_identifier(id, recursive);
            }

            // check imported scopes
            if (using_statements.size > 0) resolve_imports();
            for (auto scope : imported_scopes) {
                auto p2 = scope->get_identifier(id, false);
                if (p == nullptr) p = p2;
                else if (p2 != nullptr) {
                    // FIXME: log error identifier clash
                    // first found here: p->context()
                }
            }
            if (p != nullptr) flags -= SCOPE_SELF_CONTAINED; // we referenced an identifier outside the scope -> not self contained
        }
        return p;
    }

    // returns nullpointer if an error occurred. Use add_note() to give additional context
    virtual Shared<const CB_Type> get_type(const std::string& id, const Token_context& context, bool recursive=true)
    {
        // first: find the type identifier
        Shared<Abstx_identifier> type_id = get_identifier(id, recursive);
        if (type_id) {
            // the identifier was found -> it must have type CB_Type, and its value must be known at compile time
            if (*type_id->get_type() != *CB_Type::type) {
                log_error("non-type identifier " + type_id->toS() + " used as type", context);
                add_note("identifier defined here", type_id->context);
                status = Parsing_status::TYPE_ERROR;
                return nullptr;
            }
            if (!type_id->has_constant_value()) {
                log_error("value of type " + type_id->toS() + " is not known at compile time", context);
                add_note("identifier defined here", type_id->context);
                add_note("all types must be known at compile time");
                status = Parsing_status::COMPILE_TIME_ERROR;
                return nullptr;
            }
            return parse_type(type_id->value);
        }

        // the identifier was not found in the scope -> it might be one of the built-in types (or it doesn't exist)
        auto type = get_built_in_type(id);
        if (type == nullptr) {
            log_error("unknown identifier " + type_id->toS() + " used as type", context);
            status = Parsing_status::TYPE_ERROR;
        }
        return type;
    }

    void resolve_imports()
    {
        // @TODO: this function is not complete / doesn't work yet
        // for now, just assert that no work has to be made, then return
        ASSERT(using_statements.size == 0);
        return;

        Seq<Shared<Abstx_using>> remaining;

        while (using_statements.size > 0) {
            remaining.clear();

            for (auto us : using_statements) {
                if (us->status == Parsing_status::NOT_PARSED || us->status == Parsing_status::PARTIALLY_PARSED) {

                    if (us->status == Parsing_status::FULLY_RESOLVED) {
                        ASSERT(false, "FIXME: eval");
                        CB_Any scope;// = eval(us->subject); // FIXME: eval(value_expr)
                        if (scope.type == CB_String::type) {
                            ASSERT(false, "FIXME: string import");
                            // compile file, import its global scope
                        // } else if (scope.type == Abstx_scope::type) {
                        //     // import the scope directly
                        //     ASSERT(false, "FIXME: scope import");
                        //     // imported_scopes.add(scope.value<Abstx_scope>());
                        } else {
                            log_error("Invalid type in using statement: expected string or scope, but found "+scope.type.toS(), us->context);
                            // if nullptr, eval should already have logged error
                        }
                    } else if (us->status == Parsing_status::DEPENDENCIES_NEEDED) {
                        remaining.add(us);
                    } else {
                        ASSERT(is_error(us->status)); // ignore the error
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

    void generate_code(std::ostream& target) const override {
        ASSERT(is_codegen_ready(status));
        target << "{" << std::endl;
        for (const auto& st : statements) {
            st->generate_code(target);
        }
        target << "}" << std::endl;
    };

};


// A global scope corresponds to one compiled file. It is always static. It has no owner.

struct Global_scope : Abstx_scope
{
    std::string file_name;
    const Seq<Token> tokens; // should be treated as const

    Global_scope(Seq<Token>&& tokens) : tokens{std::move(tokens)} {
        add_built_in_types_as_identifiers();
    }

    Token_iterator iterator(int index=0) const { return Token_iterator(tokens, index); }

    Shared<Abstx_scope> parent_scope() const override { return nullptr; }
    Shared<const Global_scope> global_scope() const override { return this; }

private:
    static Seq<Owned<Abstx_identifier>> type_identifiers;
    static Token_context built_in_context;

    void add_built_in_types_as_identifiers()
    {
        built_in_context.line = 0;
        built_in_context.position = 0;
        built_in_context.file = "CB_built_in_types";

        if (type_identifiers.size == 0) {
            prepare_built_in_types();
            for (const auto& t : CB_Type::built_in_types) {
                Owned<Abstx_identifier> id = alloc(Abstx_identifier());
                id->owner = nullptr;
                id->context = built_in_context;
                id->status = Parsing_status::FULLY_RESOLVED;

                id->name = t.second->toS();
                id->uid = 0; // suppress uid
                id->value.v_type = CB_Type::type;
                id->value.v_ptr = &t.second->uid; // should be safe
                type_identifiers.add(std::move(id));
            }
        }
        for (const auto& id : type_identifiers) {
            identifiers[id->name] = Shared<Abstx_identifier>(id);
        }
    }

};



// A function scope is the same as a regular scope, but it can own its own identifiers (from the function signature). It is always dynamic.

struct Abstx_function_scope : Abstx_scope
{
    std::map<std::string, Owned<Abstx_identifier>> fn_identifiers; // id name -> id. Identifiers are owned by the function scope.

    Abstx_function_scope() : Abstx_scope((uint64_t)SCOPE_DYNAMIC) {}

    Shared<Abstx_identifier> get_identifier(const std::string& id, bool recursive=true) override
    {
        Shared<Abstx_identifier> p_local = identifiers[id];
        Shared<Abstx_identifier> p_fn = fn_identifiers[id];
        ASSERT(p_local == nullptr || p_fn == nullptr, "local name overrides not allowed"); // this should give compile error earlier
        if (p_fn != nullptr) return p_fn;
        if (p_local != nullptr) return p_local;
        return Abstx_scope::get_identifier(id, recursive);
    }

    void add_identifier(const std::string& name, Shared<const CB_Type> type) {
        Owned<Abstx_identifier> id = alloc(Abstx_identifier());
        id->name = name;
        id->value.v_type = type;
        id->owner = this;
        fn_identifiers[name] = (std::move(id));
    }

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

    Parsing_status fully_parse() override; // implemented in ../parser/statement_parser.cpp

    void generate_code(std::ostream& target) const override {
        ASSERT(is_codegen_ready(status));
        scope->generate_code(target);
    };

};












