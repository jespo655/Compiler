#include "parser.h"



std::shared_ptr<Value_expression> compile_function_call(Token_iterator& it, std::shared_ptr<Value_expression> fn_id)
{
    ASSERT(false, "NYI");
}

std::shared_ptr<Value_expression> compile_seq_indexing(Token_iterator& it, std::shared_ptr<Value_expression> seq_id)
{
    ASSERT(false, "NYI");
}

std::shared_ptr<Value_expression> compile_getter(Token_iterator& it, std::shared_ptr<Value_expression> struct_id)
{
    ASSERT(false, "NYI");
}


std::shared_ptr<Statement> read_operator_declaration(Token_iterator& it, std::shared_ptr<Value_expression> struct_id)
{
    ASSERT(false, "NYI");
}



