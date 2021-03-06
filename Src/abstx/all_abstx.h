#include "abstx.h"
#include "abstx_scope.h"
#include "statements/abstx_assignment.h"
#include "statements/abstx_c_code.h"
#include "statements/abstx_declaration.h"
#include "statements/abstx_defer.h"
#include "statements/abstx_for.h"
#include "statements/abstx_function_call.h"
#include "statements/abstx_if.h"
#include "statements/abstx_return.h"
#include "statements/abstx_statement.h"
#include "statements/abstx_using.h"
#include "statements/abstx_while.h"
#include "statements/generic_statement.h"

#include "expressions/value_expression.h"
#include "expressions/abstx_function.h"
#include "expressions/abstx_simple_literal.h"
#include "expressions/abstx_struct_literal.h"
#include "expressions/abstx_sequence_literal.h"

#include "expressions/variable_expression.h"
// #include "expressions/abstx_array_index.h" // this should maybe be handled by an operator
#include "expressions/abstx_identifier.h"
#include "expressions/abstx_identifier_reference.h"
#include "expressions/abstx_pointer_dereference.h"
#include "expressions/abstx_struct_getter.h"
