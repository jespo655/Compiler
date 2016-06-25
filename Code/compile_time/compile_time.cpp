
#include "compile_time.h"
#include "value.h"
#include <iostream>

#include "../abstx/workspace.h"

#include "../abstx/int.h"
#include "../abstx/float.h"
#include "../abstx/str.h"
#include "../abstx/seq.h"
#include "../abstx/scope.h"
#include "../abstx/function.h"

const std::shared_ptr<Workspace> compiled_workspace{new Workspace()};









/*
pass 2) dynamisk kompilering av #run (compile time execution)
utgå från en entry point, börja dynamisk kompilering
    gå igenom varje statement, försök resolva direkt
    lös dependencies för alla identifiers som används
    det enda som inte skulle kunna resolvas direkt är saker som är beroende av andra #run
    om det händer:
        spara "bokmärke" så man kan fortsätta på det ställe i kompileringen där man slutade (eller så nära som möjligt)
        sätt upp dependancy på #run_2
        kolla att #run_2 inte har dependancy på #run_1 **
        börja dynamisk kompilering av #run_2
    varje statement utförs, Values blir skapade och förstörda, etc.
*/

Value evaluate(std::shared_ptr<Literal> lit) {
    Value v(lit->get_type());
    if (auto int_l = std::dynamic_pointer_cast<Literal_int>(lit)) v.assign(lit->get_type(), int_l->value);
    if (auto uint_l = std::dynamic_pointer_cast<Literal_uint>(lit)) v.assign(lit->get_type(), uint_l->value);
    if (auto float_l = std::dynamic_pointer_cast<Literal_float>(lit)) v.assign(lit->get_type(), float_l->value);
    if (auto str_l = std::dynamic_pointer_cast<Literal_str>(lit)) {
        ASSERT(false, "using strings in compile time context overwrites the original"); // FIXME: find another solution
        v.assign(lit->get_type(), str_l->value);
    }
    if (auto seq_l = std::dynamic_pointer_cast<Literal_seq>(lit)) {
        ASSERT(false, "should never reach this point, check for sequences separately");
    }
    if (auto scope_l = std::dynamic_pointer_cast<Literal_scope>(lit)) {
        ASSERT(false, "should never reach this point, check for scopes separately");
    }
    return v;
}


Value evaluate(std::shared_ptr<Evaluated_value> ev) {
    if (auto lit = std::dynamic_pointer_cast<Literal>(ev)) return evaluate(lit);
    // fixme: add evaluate() for all other kinds of values

    ASSERT(false, "evaluate of a value of this type NYI");
}




struct Local_scope {

    std::map<std::string, Value> values; // map identifier name -> value
    std::shared_ptr<Scope> parent_scope;

    Local_scope(std::shared_ptr<Function_call> fc) {
        ASSERT(fc->fully_resolved); // this allows us to ignore expensive assertions - we know that the function call is well formed

        std::shared_ptr<Function> identity = fc->get_identity();
        parent_scope = identity->parent_scope();
        for (auto identifier : identity->in_parameters) {
            Value v = evaluate(identifier->get_type()->get_default_value());
            ASSERT(!values[identifier->name].is_allocated());
            values[identifier->name] = std::move(v);
        }
        for (auto identifier : identity->out_parameters) {
            Value v = evaluate(identifier->get_type()->get_default_value());
            ASSERT(!values[identifier->name].is_allocated());
            values[identifier->name] = std::move(v);
        }
        for (int i = 0; i < fc->arguments.size(); i++) {
            auto arg = fc->arguments[i];
            auto in_par = identity->in_parameters[i];
            std::string name = in_par->name;
            if (arg.is_named()) {
                // expensive assert: assert that the name is in the in_par list ahead
                name = arg.name;
            } else {
                // expensive assert: assert that no earlier arguments was named
            }
            ASSERT(values[name].is_allocated());
            values[name].assign(evaluate(arg.value));
        }
        // expensive assert: assert that all parameters without default value was named
    }


};




std::vector<std::shared_ptr<Literal>> hashtag_run(std::shared_ptr<Function_call> fc) {

    // set up a local scope




}




std::vector<Value> evaluate(std::shared_ptr<Function_call> fc) {

}






/*
// TODO: sequence of sequcence
std::vector<Value> evaluate(std::shared_ptr<Literal_seq> lit) {
    ASSERT(lit->type != nullptr);
    std::vector<Value> vs;
    for (auto ev : members) {
        Value v(lit->type);
        v = evaluate(ev); // log error if types doesn't match?
        vs.push_back(std::move(v));
    }
    if (vs.size() != lite->size) {
        // if evaluated size is too small, pad with default literals
        // if evaluated size is too large, remove from the end (give warning?)
    }
    return vs;
}
*/







// seq -> value
// seq har en fix size
// allokera bara så mycket minne!













/************************************************
*       Compile time execution functions
*************************************************/

void set_entry_point(std::shared_ptr<Function> entry_point, std::vector<std::shared_ptr<Evaluated_value>> arguments) {
    ASSERT(entry_point->fully_resolved);
    // FIXME: type check entry_point and arguments
    ASSERT(compiled_workspace->entry_point == nullptr); // FIXME: log_error instead
    compiled_workspace->entry_point = entry_point;
    compiled_workspace->arguments = arguments;
}


void add_c_include_path(std::string path) {
    ASSERT(!path.empty());
    compiled_workspace->c_includes.push_back(path);
}



struct Hashtag_run : Function_call {

};






std::shared_ptr<Function> get_compile_time_function(std::string id)
{
    return nullptr;
}

void execute_compile_time_function(std::string id, std::vector<std::shared_ptr<Evaluated_value>> arguments)
{

}









#include "../abstx/cast.h"
#include "../abstx/evaluated_variable.h"
#include "../abstx/function.h"

Value evaluate(std::shared_ptr<Cast> cast) {
    std::cout << "casting from " << cast->value_identifier->get_type()->toS();
    std::cout << "to " << cast->type_identifier->get_type()->toS();
    std::cout << std::endl;
    // call the correct cast function
}


Value evaluate(std::shared_ptr<Evaluated_variable> ev) {
    std::cout << "Ev variable" << std::endl;
}


Value evaluate(std::shared_ptr<Function> fn) {
    // the result is a function pointer
}


/*
Cast
Evaluated_variable
Function
Function_call // blir en lista
Literal
// Type // skapa Defined_type direkt


*/



