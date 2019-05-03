#include "abstx_function.h"

std::string Abstx_function_literal::toS() const {
    // @todo: write better toS()
    return "function";
}


void Abstx_function_literal::add_arg(bool in, Shared<Abstx_identifier> id) {
    Function_arg arg{};
    arg.identifier = id;
    if (in) in_args.add(std::move(arg));
    else out_args.add(std::move(arg));
}


Shared<const CB_Type> Abstx_function_literal::get_type()
{
    return function_identifier.get_type();
}


bool Abstx_function_literal::has_constant_value() const {
    return true;
}

const Any& Abstx_function_literal::get_constant_value() {
    ASSERT(function_identifier.value.v_type); // should be set earlier
    if (function_identifier.value.v_ptr == nullptr) function_identifier.value.v_ptr = this;
    return function_identifier.get_constant_value();
}




void Abstx_function_literal::generate_code(std::ostream& target) const {
    // generate code in the context of a value expression
    // the actual function code will be generated later, through generate_declaration()
    global_scope()->used_functions[function_identifier.uid] = (Abstx_function_literal*)this;
    return function_identifier.generate_code(target);
}

// all declaration must be generated in a global scope
// before this, all types must be defined
void Abstx_function_literal::generate_declaration(std::ostream& target, std::ostream& header) {
    // only generate the code once; this is necessary for iterative code generation
    if (!code_generated) {
        generate_declaration_internal(header, true);
        generate_declaration_internal(target, false);
    }
    code_generated = true;
};




void Abstx_function_literal::generate_declaration_internal(std::ostream& target, bool header) const {
    ASSERT(is_codegen_ready(status));

    // function declaration syntax
    target << "void "; // all cb functions returns void
    function_identifier.generate_code(target);
    target << "(";
    for (int i = 0; i < in_args.size; ++i) {
        const auto& arg = in_args[i];
        if (i) target << ", ";
        arg.identifier->get_type()->generate_type(target);
        target << " ";
        if (!arg.identifier->get_type()->is_primitive()) {
            target << "const* "; // pass primitives by value; non-primitives by const pointer
        }
        arg.identifier->generate_code(target);
    }
    if (in_args.size > 0 && out_args.size > 0) target << ", ";
    for (int i = 0; i < out_args.size; ++i) {
        const auto& arg = out_args[i];
        if (i) target << ", ";
        arg.identifier->get_type()->generate_type(target);
        target << "* "; // always pass non-cost pointer to the original value
        arg.identifier->generate_code(target);
    }
    target << ")";
    if (header) target << ";" << std::endl;
    else {
        target << " ";
        scope.generate_code(target);
    }

    // Generated code:
    // int is primitive, T is not primitive
    // default values for a, b are set during the function call (another abstx node)
    /*
    void foo(int a, T const* b, int* a, T* b) {
        // scope satatements
    }
    */
}
