#include "type_checker.h"
#include <memory>
#include "error_handler.h"
#include "assert.h"
#include <vector>
#include <string>
#include "abstx.h"

using namespace std;



// built in types
Primitive_type i8_type{"i8", 1}; // byte
Primitive_type i16_type{"i16", 2}; // short
Primitive_type i32_type{"i32", 4}; // int
Primitive_type i64_type{"i64", 8}; // long long

Primitive_type int_type = i64_type;

Primitive_type u8_type{"u8", 1}; // unsigned byte
Primitive_type u16_type{"u16", 2}; // unsigned short
Primitive_type u32_type{"u32", 4}; // unsigned int
Primitive_type u64_type{"u64", 8}; // unsigned long long

Primitive_type uint_type = u64_type;

Primitive_type float32_type{"f32", 8}; // float
Primitive_type float64_type{"f64", 8}; // double

Primitive_type float_type = float64_type;

Primitive_type string_type{"string", 8};
Primitive_type ptr_type{"pointer", 8}; // 64 bit ptr
Primitive_type bool_type{"bool", 1}; // the smallest possible.
Primitive_type type_type{"type", 8}; // ptr to the type struct
Primitive_type scope_type{"scope", 8}; // ptr to the scope struct
Primitive_type function_type{"function", 8}; // ptr to the function struct
Primitive_type void_type{"void",0};

// const Primitive_type array_type{"array", 16};

vector<shared_ptr<Primitive_type>> primitive_types
{
    shared_ptr<Primitive_type>(&i8_type),
    shared_ptr<Primitive_type>(&i16_type),
    shared_ptr<Primitive_type>(&i32_type),
    shared_ptr<Primitive_type>(&i64_type),
    shared_ptr<Primitive_type>(&int_type),
    shared_ptr<Primitive_type>(&u8_type),
    shared_ptr<Primitive_type>(&u16_type),
    shared_ptr<Primitive_type>(&u32_type),
    shared_ptr<Primitive_type>(&u64_type),
    shared_ptr<Primitive_type>(&uint_type),
    shared_ptr<Primitive_type>(&float32_type),
    shared_ptr<Primitive_type>(&float64_type),
    shared_ptr<Primitive_type>(&float_type),
    shared_ptr<Primitive_type>(&string_type),
    shared_ptr<Primitive_type>(&ptr_type),
    shared_ptr<Primitive_type>(&bool_type),
    shared_ptr<Primitive_type>(&type_type),
    shared_ptr<Primitive_type>(&scope_type),
    shared_ptr<Primitive_type>(&function_type),
    shared_ptr<Primitive_type>(&void_type)
};

shared_ptr<Type_info> get_primitive_type(const string& type_name)
{
    for (auto& p : primitive_types) {
        if (p->type_name == type_name)
            return shared_ptr<Type_info>(p.get());
    }
    return nullptr;
}
/*
shared_ptr<Type_info> get_type(const string& type_name, Scope* scope, const Token_context& context)
{
    // check if it is a primitive type
    // if it is a primitive type -> assume that is it not also an identifier in the scope (that should be caught earlier)
    // else ->
    //  check for the identifier in the scope
    //  check if the identifier identity is a type identifier
    shared_ptr<Type_info> primitive = get_primitive_type(type_name);
    if (primitive != nullptr) {
        ASSERT(identity == nullptr);
        return primitive;
    }

    shared_ptr<Typed_identifier> identity = scope->get_type(type_name);
    if (identity != nullptr) {
        if (*identity->type == type_type) {
            // we need some way to get the type
            // return identity;
        }
        return nullptr;
    }

    log_error("Type \""+type_name+"\" not found in scope",context);
}
*/






shared_ptr<Type_info> Literal::get_type()
{
    switch(literal_token->type) {
        case Token_type::INTEGER: return shared_ptr<Type_info>(&int_type);
        case Token_type::FLOAT: return shared_ptr<Type_info>(&float_type);
        case Token_type::STRING: return shared_ptr<Type_info>(&string_type);
        case Token_type::BOOL: return shared_ptr<Type_info>(&bool_type);
        default: return nullptr;
    }
}


shared_ptr<Type_info> Value_list::get_type()
{
    shared_ptr<Type_list> tl{new Type_list()};
    for (auto& value : values) {
        tl->types.push_back(value->get_type());
    }
    return tl;
}

shared_ptr<Type_info> Identifier::get_type()
{
    if (identity == nullptr) return nullptr; // should look for the identity in the local scope. If not found -> error
    return identity->get_type();
}


shared_ptr<Type_info> Infix_op::get_type()
{
    // todo: function lookup
    // in the meantime: hard coded stuff with no security at all
    if (op_token->token == "<" ||
        op_token->token == ">" ||
        op_token->token == "<=" ||
        op_token->token == ">=" ||
        op_token->token == "==" ||
        op_token->token == "!=")
        return shared_ptr<Type_info>(&bool_type);
    return lhs->get_type();
}




shared_ptr<Type_info> Getter::get_type()
{
    ASSERT(struct_identifier != nullptr);
    auto struct_type = struct_identifier->get_type();
    if (struct_type == nullptr) return nullptr;

    if (Struct_type* st = dynamic_cast<Struct_type*>(struct_type.get())) {
        for (auto& id : st->members) {
            if (id->identifier_token->token == data_identifier_token->token) {
                return id->type;
            }
        }
    } else {
        log_error("Trying to get something from something that is not a struct",data_identifier_token->context);
        return nullptr;
    }
    log_error("Could not find member in struct",data_identifier_token->context);
    return nullptr;
}

shared_ptr<Type_info> Function_call::get_type()
{
    ASSERT(function_identifier != nullptr);
    auto function_type = function_identifier->get_type();
    if (function_type == nullptr) return nullptr;

    if (Function_type* ft = dynamic_cast<Function_type*>(function_type.get())) {
        shared_ptr<Type_list> tl{new Type_list()};
        tl->types = ft->out_parameters;
        return tl;
    } else {
        // we don't have access to a context here!
        // log_error("Trying to call something that is not a function",data_identifier_token->context);
        return nullptr;
    }
}


shared_ptr<Type_info> Cast::get_type()
{
    // ASSERT(casted_type != nullptr);
    // auto type = casted_type->get_type(); // should always return "type_type"

    // problem: to get the type, we need to evaluate the Evaluated_value, that means possibly making compile time function calls
    // we don't want that.
    // solution: only accept cast to identifiers instead of to generic Evaluated_values
    cerr << "Cast::get_type() nyi" << endl;
    return nullptr; // todo
}

shared_ptr<Type_info> Array_lookup::get_type()
{
    ASSERT(array_identifier != nullptr);
    auto array_type = array_identifier->get_type();
    if (array_type == nullptr) return nullptr;

    if (Array_type* at = dynamic_cast<Array_type*>(array_type.get())) {
        return shared_ptr<Array_type>(at);
    } else {
        // we don't have access to a context here!
        // log_error("Trying to call something that is not a function",data_identifier_token->context);
        return nullptr;
    }

    cerr << "Array_lookup::get_type() nyi" << endl;
    return nullptr; // todo
}

shared_ptr<Type_info> Type_info::get_type()
{
    return shared_ptr<Type_info>(&type_type);
}


shared_ptr<Type_info> Scope::get_type()
{
    return shared_ptr<Type_info>(&scope_type);
}

shared_ptr<Type_info> Function::get_type()
{
    return shared_ptr<Type_info>(&function_type);
}

shared_ptr<Type_info> Range::get_type()
{
    return start->get_type(); // assuming that the end has the same type
}






// to be checked:

// dynamic statements
// static statements


// identifiers:
//      check for identity in scope. If not found - scope may just have not been pulled in yet

// function call
//      check that the function is defined

// infix op
//      check that the operator is defined
//      at get_type() : check for the defined operator, return its return value

// cast
//      check that the cast operator is defined
//      at get_type() :


























/*
1) Regular named identifier
    a := 2; // a is a variable identifier

2) Function identifier
    foo := fn(){}; // foo är en funktion
    bar : fn(); // bar är en funktion vars body inte är definierad än

3) Type identifier
    vec3 := struct { x,y,z : float,float,float; }; // vec3 är en type identifier
    // f : typeof(fn()); // kanske: f är funktionstypen fn()

Varje identifier kan endast vara definierad en gång.
Detta kanske dock inte har hänt när dependency sätts upp!
*/


/*
check local scope - if found, great! return.
more than one copy cannot exist in the scope. (except for functions? todo*)

check all imported scopes.
If found in more than one branch -> error "Ambiguous reference to identfier x" "defined here:" "required from here"
*/

// returns nullptr if the identifier could not be not found.
//      that is an error if all using-statements have been resolved.
// if the same identifier was found in several import branches -> ambiguous reference. The first reference found is returned.
//      that is always an error.
shared_ptr<Typed_identifier> Scope::get_identifier(const string& identifier_name, const Token_context& context)
{
    shared_ptr<Typed_identifier> tid;
    bool ambiguous = false;
    if (get_identifier(identifier_name, tid, context, ambiguous)) {
        if (tid == nullptr) log_error("Identifier \""+identifier_name+"\"could not be found",context);
        return nullptr;
    }
    return tid;
}


bool Scope::get_identifier(const string& identifier_name, shared_ptr<Typed_identifier>& identifier, const Token_context& context, bool& ambiguous)
{
    // first check local identifiers
    // then check imports
    for (shared_ptr<Typed_identifier>& id : identifiers) {
        if (id->identifier_token->token == identifier_name) {
            // match!
            if (identifier != nullptr) {
                if (!ambiguous) {
                    log_error("Ambiguous reference to identifier \""+identifier_name+"\".",context);
                    add_note("Declared here: ",identifier->identifier_token->context);
                    ambiguous = true;
                }
                add_note("And here: ",id->identifier_token->context);
                return true;
            }
            identifier = id;
            return false;
        }
    }

    for (Scope* imported_scope : imported_scopes) {
        imported_scope->get_identifier(identifier_name, identifier, context, ambiguous);
    }

    if (ambiguous) return true;
    if (identifier == nullptr) return true;
    return false;
}

shared_ptr<Type_info> Scope::get_type(const string& type_name, const Token_context& context)
{
    // 1) check primitive types.
    auto primitive = get_primitive_type(type_name);
    if (primitive != nullptr) return primitive;

    // 2) check if the identifier actually can be found in the scope.
    // This will log all possible errors.
    // if this doesn't return nullptr, then we are certain that we can find the type witout any errors.
    if (get_identifier(type_name, context) == nullptr) return nullptr;

    // 3) find the type
    auto type = get_type_no_checks(type_name);
    ASSERT(type != nullptr); // since we found the identifier, we must be able to find the type
    return type;
}

// get_type_no_checks assumes that we won't get any errors while searching for the type.
shared_ptr<Type_info> Scope::get_type_no_checks(const string& type_name)
{
    // first check local types
    // then check imports
    for (shared_ptr<Type_info>& t : types) {
        if (t->get_type_id() == type_name) {
            // match!
            return t;
        }
    }

    for (Scope* imported_scope : imported_scopes) {
        auto t = imported_scope->get_type_no_checks(type_name);
        if (t != nullptr) return t;
    }

    return nullptr;
}

















/*
void add_dependency(Dynamic_statement* statement, Identifier* identifier, Scope* scope)
{
    if (Static_scope* ss = dynamic_cast<Static_scope*>(scope.get())) {
        ss->dependencies.push_back(pair(statement,identifier));
    } else {
        for (Scope* s : scope.imported_scopes) {
            add_dependency(statement,identifier,s);
        }
    }
}

void remove_dependancies(Dynamic_statement* statement, Scope* scope)
{
    if (Static_scope* ss = dynamic_cast<Static_scope*>(scope.get())) {
        for (int i = ss->dependencies.size()-1; i >= 0; --i) {
            if (ss->dependencies[i].first == statement) {
                ss->dependencies.erase(i);
            }
        }
    } else {
        for (Scope* s : scope.imported_scopes) {
            remove_dependancies(statement,s);
        }
    }
}

void resolve_identifier(Typed_identifier* identifier, Scope* scope)
{
    for (Typed_identifier& ids : scope.identifiers) {
        // check if it's alread in the scope. If not -> add it.
        // If there - assert that it not already has a type - This function should only be called once per identifier.
    }
    vector<Dynamic_statement*> statements_to_be_resolved{};
    if (Static_scope* ss = dynamic_cast<Static_scope*>(scope.get())) {
        for (auto p : ss->dependencies) {
            // if the identifier match -> reduce the statement's dependency count by 1
            // if dep.count reaches 0 -> add to statements_to_be_resolved
        }
    }
    for (Dynamic_statement* ds : statements_to_be_resolved) {
        ASSERT(ds->dependencies == 0);
        // Try to resolve. If it's not possible -> some error in the compiler
    }
}
*/


// TODO: check this case:
/*
s1:={
    a : int;
    s2:={
        b := a;     // when we reach this point, a will be defined in s1 but not s2.
        a : float;  // this is closer to b than the previous definition -> b should be a float!
    };
};
*/













// Try to resolve the local scope first. If fully resolved and identifier not found -> add dependencies on other scopes
//      Check level for level
//      Add identifiers to the scope even if they can't be resolved, so we know they exist
// Go through all scopes and set up dependencies and add all identifiers BEFORE resolving depending statements
// -> When setting up dependencies, we can KNOW which scope we want to get the identifier from
// Can also give error if we find multiple declarations of the same identifier on the same level







/*
1) Go through all static scopes in the workspace
    add identifiers to the scopes
    store a list of unresolved statements to compile (for example, the bodies of functions)

2) Import scopes with "using" (has to be done before step 3)

3) (loop) Resolve unresolved identifiers.

*/
