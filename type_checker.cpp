#include "type_checker.h"
#include <memory>
#include "error_handler.h"
#include "assert.h"
#include <vector>
#include <string>

using namespace std;


// lookup functions
// TODO: make these functions a search in a sorted vector of tokens @optimization

bool is_primitive_type(const Token& t)
{
    if (t.type != Token_type::IDENTIFIER) return false;

    if (t.token == "u8") return true;
    if (t.token == "u16") return true;
    if (t.token == "u32") return true;
    if (t.token == "u64") return true;
    if (t.token == "i8") return true;
    if (t.token == "i16") return true;
    if (t.token == "i32") return true;
    if (t.token == "i64") return true;
    if (t.token == "int") return true;
    if (t.token == "float") return true;
    if (t.token == "string") return true;

    return false;
}









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
ASSERT multiple copies are not found.

check all imported scopes.
If found in more than one branch -> error "Ambiguous reference to identfier x" "defined here:" "required from here"
*/


// identifier becomes nullptr if the identifier could not be not found.
// That is not an error, just an indication that you should set up a dependency for that variable.
// Returns true if the same identifier was found in several import branches -> ambiguous reference
bool find_identifier(const std::string& identifier_name, Scope* scope, std::shared_ptr<Typed_identifier>& identifier, const Token_context& context)
{
    identifier = nullptr;
    // first check local identifiers
    // then check imports
    for (shared_ptr<Typed_identifier>& id : scope->identifiers) {
        if (id->identifier_token->token == identifier_name) {
            // match!
            ASSERT(identifier == nullptr); // a scope cannot have several identifiers of the same name // or maybe it can? @overloading
            identifier = id;
        }
    }
    if (identifier != nullptr) return false;

    bool errors = false;
    bool ambiguous = false;
    for (Scope* imported_scope : scope->imported_scopes) {
        shared_ptr<Typed_identifier> id = nullptr;
        if (find_identifier(identifier_name,imported_scope,id,context)) errors = true;
        if (id != nullptr) {
            if (identifier != nullptr) {
                if (!ambiguous) {
                    log_error("Ambiguous reference to identifier \""+identifier_name+"\".",context);
                    add_note("Declared here: ",identifier->identifier_token->context);
                    ambiguous = true;
                }
                add_note("And here: ",id->identifier_token->context);
                errors = true;
            } else {
                identifier = id; // ok
            }
        }
    }
    return errors;
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
