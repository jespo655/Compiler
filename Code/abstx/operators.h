#pragma once

#include "function.h"
#include <sstream>


struct Operator : Function_call {

    std::string function_name;

    std::shared_ptr<Function> get_identity()
    {
        if (identity == nullptr) {
            auto scope = parent_scope();
            ASSERT(scope != nullptr);
            identity = scope->get_function(get_mangled_identifier());
            fully_resolved = true;
        }
        return identity;
    }

    std::string toS() = 0;

};



/*
An infix operator is just another way to write a function that takes two arguments and returns one.

infix_operator+(1, 2); // returns 3
1 + 2; // returns 3
*/
struct Infix_operator : Operator {

    std::string get_mangled_identifier() const override
    {
        ASSERT(arguments.size() == 2)
        auto lhs = arguments[0];
        auto rhs = arguments[1];
        ASSERT(lhs != nullptr);
        ASSERT(rhs != nullptr);

        auto lhs_type = lhs->get_type();
        auto rhs_type = rhs->get_type();
        ASSERT(lhs_type != nullptr);
        ASSERT(rhs_type != nullptr);

        std::ostringstream oss;
        oss << "infix_operator " << function_name << "(";
        oss << lhs_type << ", " << rhs_type;
        oss << ")";
        return oss.str();
    }

    std::string toS() const override
    {
        ASSERT(arguments.size() == 2)
        auto lhs = arguments[0];
        auto rhs = arguments[1];
        ASSERT(lhs != nullptr);
        ASSERT(rhs != nullptr);

        std::ostringstream oss;
        oss << "infix_operator " << function_name << "(";
        oss << lhs->toS() << ", " << rhs->toS();
        oss << ")";
        return oss.str();
    }


};



/*
A prefix operator is just another way to write a function that takes one argument and returns one.

prefix_operator++(1); // returns 2
++1; // returns 2
*/
struct Prefix_operator : Statement {

    std::string get_mangled_identifier() const override
    {
        ASSERT(arguments.size() == 1)
        auto argument = arguments[0];
        ASSERT(argument != nullptr);

        auto arg_type = argument->get_type();
        ASSERT(arg_type != nullptr);

        std::ostringstream oss;
        oss << "prefix_operator " << function_name << "(" << arg_type << ")";
        return oss.str();
    }

    std::string toS() const override
    {
        ASSERT(arguments.size() == 1)
        auto argument = arguments[0];
        ASSERT(argument != nullptr);

        std::ostringstream oss;
        oss << "prefix_operator " << function_name << "(" << argument->toS() << ")";
        return oss.str();
    }

};



/*
A suffix operator is just another way to write a function that takes one argument and returns one.

i:=1; suffix_operator++(i); // returns 1. i is now 2.
i:=1; i++; // returns 1. i is now 2.
*/
struct Suffix_operator : Statement {

    std::string get_mangled_identifier() const override
    {
        ASSERT(arguments.size() == 1)
        auto argument = arguments[0];
        ASSERT(argument != nullptr);

        auto arg_type = argument->get_type();
        ASSERT(arg_type != nullptr);

        std::ostringstream oss;
        oss << "suffix_operator " << function_name << "(" << arg_type << ")";
        return oss.str();
    }

    std::string toS() const override
    {
        ASSERT(arguments.size() == 1)
        auto argument = arguments[0];
        ASSERT(argument != nullptr);

        std::ostringstream oss;
        oss << "suffix_operator " << function_name << "(" << argument->toS() << ")";
        return oss.str();
    }

};






/*

// Generates c-code just like functions does.

*/