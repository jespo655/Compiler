#include "abstx_function_call.h"

namespace Cube {

void Abstx_function_call::generate_code(std::ostream& target) const override
{
    ASSERT(is_codegen_ready(status));
    function_pointer->generate_code(target);
    target << "(";
    for (int i = 0; i < in_args.size; ++i) {
        const auto& arg = in_args[i];
        if (i) target << ", ";
        if (!arg->get_type()->is_primitive()) {
            target << "&"; // pass primitives by value; non-primitives by const pointer
        }
        arg->generate_code(target);
    }
    if (in_args.size > 0 && out_args.size > 0) target << ", ";
    for (int i = 0; i < out_args.size; ++i) {
        const auto& arg = out_args[i];
        if (i) target << ", ";
        target << "&"; // always pass non-cost pointer to the original value
        arg->generate_code(target);
    }
    target << ");" << std::endl;
}


bool Abstx_function_call::get_args_as_any(Seq<Shared<const Any>>& args) {
    args.clear();
    for (auto& expr : in_args) {
        if (!expr->has_constant_value()) {
            LOG("in arg expr at " << expr->context.toS() << " is not constant!");
            return false;
        }
        args.add(&expr->get_constant_value());
    }
    for (auto& expr : out_args) {
        if (!expr->has_constant_value()) {
            LOG("out arg expr at " << expr->context.toS() << " is not constant!");
            return false;
        }
        args.add(&expr->get_constant_value());
    }
    return true;
}





std::string Abstx_function_call_expression::toS() const override { return "function call expression"; }


Shared<const CB_Type> Abstx_function_call_expression::get_type() override {
    ASSERT(false, "Abstx_function_call_expression::get_type() not allowed since functions can have several types - check funcion_call->out_args instead");
}

void Abstx_function_call_expression::generate_code(std::ostream& target) const override {
    ASSERT(function_call);
    function_call->generate_code(target);
}

bool Abstx_function_call_expression::has_constant_value() const override {
    // @todo: function call has constant value if all arguments are consants, and if the function scope is self contained (no bi effects)
    return false;
}

const Any& Abstx_function_call_expression::get_constant_value() override {
    static const Any no_value;
    return no_value;
}

void Abstx_function_call_expression::finalize() override {
    // the function call should be finalized by the scope finalizer
    ASSERT(function_call != nullptr);
    if (is_error(status) || is_codegen_ready(status)) return;

    status = function_call->fully_parse();
    LOG("Abstx_function_call_expression at" << context.toS() << " has status " << status);
}

}