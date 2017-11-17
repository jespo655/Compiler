

// this is a comment

/* this is a block comment */

/* Block coments can be nested
/* And cover several lines */*/



// Declaration
// identifier_name : type_name = value ;
// identifier_name := value ; // the type will be inferred from the value

a : int; // will be given the default value 0
b : int = 1;
c := 2;  // the type will be inferred to int

// More than one identifier can be declared at the same time.
i1, i2, i3 := 1, 2, 3; // Lhs and Rhs must contain the same number of elements.


// Declaration using other variables is ok
d := e;
e := 2; // this is ok. Note that e is declared after d.



// The scope outside of all functions is called a static scope.
// In a static scope, redefinitions of variables are illegal.

// a = 2; // illegal, assignment is not allowed in static scope
// b : float; // illegal, b is already declared as int



// Some useful error messages:
// e : ; // forgot to specify a type -> error "Unable to infer the type of 'e'"


// Circular assignment
// g := h; // this is not ok. Should give error "Unable to infer the type of 'g'"
// h := g; // this is not ok. Should give error "Unable to infer the type of 'h'"



