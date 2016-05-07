


a := 2; // a becomes a int
b := 2.0; // b becomes a float

c := a; // c becomes the same type as a, which is int

d : int = a; // ok
// e : int = b; // not ok, implicit cast from int to float is not allowed
e : int = (b)_int; // ok, b will be casted to int before assigned to e
f := (b)_int; // f becomes int, becuase that's the type exposed from the cast

g := 0.2_int; // g becomes the int 0. The parenthesis is not necessary around literals
g := (0.2_int)_float; // g becomes the float 0.0, because the casts to int and back truncates the literal 0.2




// For more casts, see the constructor demo.



