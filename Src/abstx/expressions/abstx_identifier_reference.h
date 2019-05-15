
#include "variable_expression.h"

/*
An identifier reference is a variable expreesion, referencing a identifier defined somewhere else.
The reference is owned by the statement it is in.

This class is mostly here so the same identifier can be used and "owned" by several places, while the original identifier
  is only declared once (and is owned by that declaration statement).
*/

struct Abstx_identifier;

struct Abstx_identifier_reference : Variable_expression {
    std::string name = "";
    Shared<Abstx_identifier> id = nullptr;

    std::string toS() const override;

    virtual Shared<const CB_Type> get_type() override;
    bool has_constant_value() const override;
    const Any& get_constant_value() override;

    void generate_code(std::ostream& target, const Token_context& context) const override;
    void finalize() override;
};
