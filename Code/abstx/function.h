#pragma once

#include "type.h"               // Type_fn
#include "evaluated_value.h"    // Function
#include "statement.h"            // Function_call

#include "identifier.h"         // ip->toS();
#include "scope.h"              // function_scope->debug_print(os, recursive);

#include <string>
#include <vector>
#include <sstream>

/*
function: type is serialized and used as identifier

map from serialized fn name+type to abs_stx_node
if default values: the same fn is mapped several times with a different number of arguments

fn_node has a list of in and out parameters
each one has:
    * name (could be automatically generated for return values)
    * type
    * default value (can be inferred from the type) (only literals allowed)

The type of the parameters can be used to infer the function type
The type of the in parameters are used for type checking


when looking up functions, check for perfect matches first
then generic functions
then implicit cast one thing at the time, check for close matches first
*/

struct Type_fn : Type {

    std::vector<std::shared_ptr<Type>> in_types;
    std::vector<std::shared_ptr<Type>> out_types;

    std::string toS() const
    {
        std::ostringstream oss;
        oss << "fn";
        if (!in_types.empty()) {
            print_parameter_list(oss, in_types);
        }
        if (!out_types.empty()) {
            oss << "->";
            print_parameter_list(oss, out_types, out_types.size() > 1);
        }
        return oss.str();
    }

private:
    void print_parameter_list(std::ostringstream& os, const std::vector<std::shared_ptr<Type>>& types, bool parens = true) const
    {
        if (parens) os << "(";
        bool first = true;
        for (auto type : types) {
            if (!first) os << ",";
            os << type->toS();
            first = false;
        }
        if (parens) os << ")";
    }
};





struct Function : Evaluated_value {

    std::vector<std::shared_ptr<Identifier>> in_parameters;
    std::vector<std::shared_ptr<Identifier>> out_parameters;

    // the function_scope should have dynamic = true
    // and all the in and out parameters should be in the scope
    // the unnamed out parameters should get the name __rparam_N,
    // with N replaced with an unique identifier number
    std::shared_ptr<Scope> function_scope;

    void debug_print(Debug_os& os, bool recursive=true) const override
    {
        os << toS() << std::endl;
        if (recursive && function_scope != nullptr) {
            os.indent();
            function_scope->debug_print(os, recursive);
            os.unindent();
        }
    }

    std::string toS() const override
    {
        std::ostringstream oss;
        oss << "fn";
        if (!in_parameters.empty()) {
            print_parameter_list(oss, in_parameters);
        }
        if (!out_parameters.empty()) {
            oss << "->";
            print_parameter_list(oss, out_parameters, out_parameters.size() > 1);
        }
        return oss.str();
    }

    std::shared_ptr<const Type> get_type() const override
    {
        std::shared_ptr<Type_fn> type{new Type_fn()};
        for (auto id : in_parameters) {
            ASSERT(id->type != nullptr);
            type->in_types.push_back(id->type);
        }
        for (auto id : out_parameters) {
            ASSERT(id->type != nullptr);
            type->out_types.push_back(id->type);
        }
        return type;
    }

private:
    void print_parameter_list(std::ostringstream& oss, const std::vector<std::shared_ptr<Identifier>>& parameters, bool parens = true) const
    {
        if (parens) oss << "(";
        bool first = true;
        for (auto ip : parameters) {
            if (!first) oss << ",";
            oss << ip->toS();
            first = false;
        }
        if (parens) oss << ")";
    }


};








struct Function_call : Statement {

    std::string function_name;
    std::vector<std::shared_ptr<Evaluated_value>> arguments;

    std::shared_ptr<Function> get_identity()
    {
        // FIXME: check for identifier, verify that it really is a function we are calling
        // Then check for perfect match with types
        // If generic, generate a new type if needed

        if (identity == nullptr) {
            auto scope = parent_scope();
            ASSERT(scope != nullptr);
            identity = scope->get_function(get_mangled_identifier());
            fully_resolved = true;
        }
        return identity;
    }

    // get_mangled_identifier() should return the mangled operator name
    // with types - that string will be used for function lookup
    virtual std::string get_mangled_identifier() const { return function_name; } // FIXME: also include types

private:
    std::shared_ptr<Function> identity; // points to the function node that represents this function

};


















/* // TODO: generic functions
struct Generic_function : Function {

};
*/



