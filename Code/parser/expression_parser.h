#pragma once

#include "token_iterator.h"
#include <memory>

struct Scope;
struct Value_expression;
struct Variable_expression;

/*
Parsing expressions are the most complex part of the parsing process.
There are so many special cases and complex complications that this deserves to be in a separate file.
*/



// These parsing functions start at the current iterator position, and reads one expression.
// The parsing will stop at the first encountered ',', ')', or any other unexpected token (without logging an error).
// When the function returns, the iterator will point to said unexpected token.

// The parent scope is necessary for
std::shared_ptr<Value_expression> parse_value_expression(Token_iterator& it, std::shared_ptr<Scope> parent_scope, int min_operator_prio=0);
std::shared_ptr<Variable_expression> parse_variable_expression(Token_iterator& it, std::shared_ptr<Scope> parent_scope);





/*
P   prefix operator
S   suffix operator
I   infix operator

High priority special operators:
()  function call (as suffix), or a sequence of expressions (surrounding other expressions)
[]  array indexing (as suffix), or an array literal (surrounding other expressions)
 MAYBE: || absolute value (surrounding other expressions that evaluate to numbmers)
.   getter



Valid expressions: (where expr is an expression and expr_list is a comma separated list of expressions)

literal             (bool, int, float, string)
identifier
P expr
expr S
expr I expr
expr(expr_list)
(expr_list)
expr[expr]
expr . identifier


A value expression is a variable expression if:
    * Getter
    * Array indexing
    * Identifier
    * Function or operator that returns a pointer







Complex expressions

P P P expr              // P(P(P(expr)))
expr S S S              // (((expr)S)S)S
expr I1 expr I2 expr    // (expr I1 expr) I2 expr if I1.prio>=I2.prio, else expr I1 (expr I2 expr)
P expr I expr           // (P expr) I1 expr if P.prio>=I.prio, else P(expr I expr)
expr I expr S           // (expr I1 expr) S if I.prio>=S.prio, else expr I (expr S)
P expr S                // (P expr) S if P.prio>=I.prio, else P(expr S)
expr S I expr           // (expr S) I1 expr
expr I P expr           // expr I1 (P expr)

Even more complex: if @ is both P, I and S at the same time:

expr @ expr             // @ is I
@ expr                  // @ is P
expr @                  // @ is S
expr @1 @2 expr         // @1=I, @2=P OR @1=S, @2=I. If both these combinations are possible, then logs an error





Error handling:

expr P I expr           // syntax error, "P can't modify an operator"
expr SI PI expr         // SI is suffix and infix, and PI is infix and prefix. syntax error "Unable to resolve order of operators"









inbyggda operatorer:

infix_operator .. := fn(low:$N1, high:$N2)->r:range
{
    #run assert_number(N1);
    #run assert_number(N2);
    r.low = low.float; // guaranteed to work after the assert
    r.high = high.float;
};



infix



*/

























// Backup-functions if genereics doesn't work out:

// infix_operator .. := fn(low:float, high:float)->r:range { r.low = low; r.high = high; };
// infix_operator .. := fn(low:float, high:int)->r:range { r.low = low; r.high = high.float; };
// infix_operator .. := fn(low:int, high:int)->r:range { r.low = low.float; r.high = high.float; };



