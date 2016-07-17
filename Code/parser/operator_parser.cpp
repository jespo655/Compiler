#include "parser.h"




// syntax:
// infix_operator op := fn_expr;
// infix_operator op := fn_expr, prio_expr;
std::shared_ptr<Declaration_statement> read_operator_declaration(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    bool infix = it->token == "infix_operator";
    bool prefix = it->token == "prefix_operator";
    ASSERT(infix xor prefix);
    it.assert(Token_type::KEYWORD);

    // read operator token

    // expect ':'
    // expect '='

    // read expr, expect the type to be a function type with exactly 1 (prefix) or 2 (infix) arguments
    // optional ',' token, then expect expression which evaluates to exactly 1 int value

    ASSERT(false, "NYI");

}
