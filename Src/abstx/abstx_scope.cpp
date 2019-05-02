
#include "abstx_scope.h"
#include "statements/abstx_function_call.h"
// #include "statements/abstx_using.h"
// #include "expressions/abstx_identifier.h"
// #include "../utilities/flag.h"
// #include "../types/cb_string.h"



void Abstx_scope::debug_print(Debug_os& os, bool recursive=true) const override
{
    os << "{ // " << toS() << std::endl;
    os.indent();
    for (auto& s : statements) {
        ASSERT(s != nullptr);
        s->debug_print(os, recursive);
    }
    os.unindent();
    os << "}" << std::endl;
}

std::string Abstx_scope::toS() const override { return dynamic()? "scope(d)" : "scope(s)"; }

virtual Shared<Abstx_identifier> Abstx_scope::get_identifier(const std::string& id, const Token_context& context, bool recursive=true)
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
                log_error("multiple definition of identifier " + id, context);
                add_note("previously defined here", p->context);
                status = Parsing_status::COMPILE_TIME_ERROR;
                return nullptr;
            }
        }
        if (p != nullptr) flags -= SCOPE_SELF_CONTAINED; // we referenced an identifier outside the scope -> not self contained
    }
    return p;
}

virtual Shared<const CB_Type> Abstx_scope::get_type(const std::string& id, const Token_context& context, bool recursive=true)
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

void Abstx_scope::resolve_imports()
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

void Abstx_scope::generate_code(std::ostream& target) const override {
    ASSERT(is_codegen_ready(status), status << " " << toS() << " at " << context.toS());
    target << "{" << std::endl;
    for (const auto& st : statements) {
        st->generate_code(target);
    }
    target << "}" << std::endl;
};

Parsing_status Abstx_scope::fully_parse() {
    if (is_error(status) || is_in_progress(status) || is_codegen_ready(status)) return status;
    LOG("fully parsing scope at " << context.toS() << " with status " << status);

    // read statements if we haven't done that yet. Non-global scopes are always dynamic.
    auto it = parse_begin();
    status = read_dynamic_scope_statements(it, this);

    ASSERT(is_error(status) || status == Parsing_status::FULLY_RESOLVED);
    return status;
}





Parsing_status Global_scope::fully_parse() {
    if (is_error(status) || is_in_progress(status) || is_codegen_ready(status)) return status;
    LOG("fully parsing scope at " << context.toS() << " with status " << status);

    // read statements if we haven't done that yet. Global scopes are always static.
    auto it = parse_begin();
    status = read_static_scope_statements(it, this);

    ASSERT(is_error(status) || status == Parsing_status::PARTIALLY_PARSED);

    if (!is_error(status)) {
        for (auto& s : statements) {
            s->fully_parse();
            if (is_error(s->status) && !is_fatal(status)) status = s->status;
        }
    }

    if (!is_error(status)) status = Parsing_status::FULLY_RESOLVED;

    // @TODO: find entry point
    // fully_parse the entry point's function scope

    return status;
}

void Global_scope::add_built_in_types_as_identifiers()
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



Shared<Abstx_identifier> Abstx_function_scope::get_identifier(const std::string& id, const Token_context& context, bool recursive=true) override
{
    Shared<Abstx_identifier> p_local = identifiers[id];
    Shared<Abstx_identifier> p_fn = fn_identifiers[id];
    ASSERT(p_local == nullptr || p_fn == nullptr, "local name overrides not allowed"); // this should give compile error earlier @TODO: check if this should be a logged error instead
    if (p_fn != nullptr) return p_fn;
    if (p_local != nullptr) return p_local;
    return Abstx_scope::get_identifier(id, context, recursive);
}