#include "abstx/abstx.h"
#include "abstx/assignment.h"
#include "abstx/bool.h"
#include "abstx/cast.h"
#include "abstx/declaration.h"
#include "abstx/defer.h"
#include "abstx/evaluated_value.h"
#include "abstx/evaluated_variable.h"
#include "abstx/float.h"
#include "abstx/for.h"
#include "abstx/function.h"
#include "abstx/getter.h"
#include "abstx/identifier.h"
#include "abstx/if.h"
#include "abstx/int.h"
#include "abstx/literal.h"
#include "abstx/operators.h"
#include "abstx/pointer.h"
#include "abstx/range.h"
#include "abstx/return.h"
#include "abstx/scope.h"
#include "abstx/seq.h"
#include "abstx/statement.h"
#include "abstx/str.h"
#include "abstx/struct.h"
#include "abstx/type.h"
#include "abstx/using.h"
#include "abstx/while.h"
#include "abstx/workspace.h"

#include "compile_time/compile_time.h"
#include "compile_time/type_checker.h"

#include "utilities/assert.h"
#include "utilities/debug.h"
#include "utilities/error_handler.h"
#include "utilities/unique_id.h"

// int main() {}