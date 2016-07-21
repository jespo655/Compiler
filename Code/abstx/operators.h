#pragma once

#include "function.h"
#include "value_expression.h"
#include "type.h"
#include <sstream>
#include <vector>








/*
FIXME: rewrite this!

*/

struct Prefix_expr : Value_expression
{
    std::shared_ptr<Identifier> op_id;
    std::shared_ptr<Value_expression> expr;

    std::shared_ptr<Type> get_type()
    {
        if (expr == nullptr || op_id == nullptr) return nullptr;
        auto expr_t = expr->get_type();
        if (expr_t == nullptr) return nullptr;

        // FIXME: check for the correct operator overload from parent_scope
        // Return its return value
        ASSERT(false, "NYI");
        return nullptr;
    }

    std::string toS() const override { return op_id->toS() + "(" + expr->toS() + ")"; }

};



struct Infix_expr : Value_expression
{
    std::shared_ptr<Identifier> op_id;
    std::shared_ptr<Value_expression> lhs;
    std::shared_ptr<Value_expression> rhs;

    std::shared_ptr<Type> get_type()
    {
        if (lhs == nullptr || rhs == nullptr || op_id == nullptr) return nullptr;
        // auto expr_t = expr->get_type();
        // if (expr_t == nullptr) return nullptr;

        // FIXME: check for the correct operator overload from parent_scope
        // Return its return value
        ASSERT(false, "NYI");
        return nullptr;
    }

    std::string toS() const override { return op_id->toS() + "(" + lhs->toS() + ", " + rhs->toS() + ")"; }

};



struct Type_operator : Type_fn
{
    // FIXME: different prios is needed for prefix and infix
    int get_prio() const { ASSERT(false, "NYI"); }

    std::string toS(void const * value_ptr, int size=0) const override {
        ASSERT(size == 0 || size == byte_size());
        std::ostringstream oss;
        void* fn_ptr = *(void**)value_ptr;
        oss << "operator pointer(" << fn_ptr << ")";
        return oss.str();
    }

    bool is_operator_type() const override { return true; }
    // int get_operator_prio() const override { return get_prio(); }

};


























/*
An operator is a special case of a function call, with it's own syntax.
The function_identifier has to be either a single word (identifier), or a symbol
*/



// This class is not meant to be used as a value, just as a way to allow symbols as Operator::function_identifier
struct Operator_symbol : Value_expression {

    std::string symbol;

    std::shared_ptr<Type> get_type()
    {
        ASSERT(false, "Operator_symbol::get_type() should never be called.");
        return nullptr;
    }

    std::string toS() const override { return symbol; }

};



struct Operator : Function_call {

    int priority=0; // higher priority goes first

    // A more slimmed down version of Function_call's get_identity(). Only check scope->get_function() directly, skip identifiers.
    std::shared_ptr<Function> get_identity() override
    {
        if (identity == nullptr) {
            auto scope = parent_scope();
            ASSERT(scope != nullptr);
            identity = scope->get_function(get_mangled_identifier());
            status = Parsing_status::FULLY_RESOLVED;
        }
        return identity;
    }

    std::string get_operator_name() const {
        ASSERT(function_identifier != nullptr);
        if (auto op = std::dynamic_pointer_cast<Operator_symbol>(function_identifier)) return op->symbol;
        if (auto id = std::dynamic_pointer_cast<Identifier>(function_identifier)) return id->name;
        ASSERT(false, "Operator::function_identifier can only be Operator_symbol or Identifier.");
    }

    std::string toS() const override = 0;

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
        auto lhs = arguments[0].value;
        auto rhs = arguments[1].value;
        ASSERT(lhs != nullptr);
        ASSERT(rhs != nullptr);
        auto lhs_type = lhs->get_type();
        auto rhs_type = rhs->get_type();
        ASSERT(lhs_type != nullptr);
        ASSERT(rhs_type != nullptr);

        std::ostringstream oss;
        oss << "_infix_operator_ " << get_operator_name()
            << "_" << lhs_type << "_" << rhs_type;
        return oss.str();
    }

    std::string toS() const override
    {
        ASSERT(arguments.size() == 2)
        auto lhs = arguments[0].value;
        auto rhs = arguments[1].value;
        ASSERT(lhs != nullptr);
        ASSERT(rhs != nullptr);

        std::ostringstream oss;
        oss << "infix_operator " << get_operator_name() << "(";
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
struct Prefix_operator : Operator {

    std::string get_mangled_identifier() const override
    {
        ASSERT(arguments.size() == 1)
        auto argument = arguments[0].value;
        ASSERT(argument != nullptr);

        auto arg_type = argument->get_type();
        ASSERT(arg_type != nullptr);

        std::ostringstream oss;
        oss << "_prefix_operator_" << get_operator_name()
            << "_" << arg_type;
        return oss.str();
    }

    std::string toS() const override
    {
        ASSERT(arguments.size() == 1)
        auto argument = arguments[0].value;
        ASSERT(argument != nullptr);

        std::ostringstream oss;
        oss << "prefix_operator " << get_operator_name() << "(" << argument->toS() << ")";
        return oss.str();
    }

};



/*
A suffix operator is just another way to write a function that takes one argument and returns one.

i:=1; suffix_operator++(i); // returns 1. i is now 2.
i:=1; i++; // returns 1. i is now 2.
*/
struct Suffix_operator : Operator {

    std::string get_mangled_identifier() const override
    {
        ASSERT(arguments.size() == 1)
        auto argument = arguments[0].value;
        ASSERT(argument != nullptr);

        auto arg_type = argument->get_type();
        ASSERT(arg_type != nullptr);

        std::ostringstream oss;
        oss << "_suffix_operator_" << get_operator_name()
            << "_" << arg_type;
        return oss.str();
    }

    std::string toS() const override
    {
        ASSERT(arguments.size() == 1)
        auto argument = arguments[0].value;
        ASSERT(argument != nullptr);

        std::ostringstream oss;
        oss << "suffix_operator " << get_operator_name() << "(" << argument->toS() << ")";
        return oss.str();
    }

};




/*

// Generates c-code just like Function_call does.

*/






/*
struct Type_prefix_op : Type_operator
{

};
struct Type_suffix_op : Type_operator
{

};
struct Type_infix_op : Type_operator
{

};
*/