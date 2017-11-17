// Operator overloading


// Function calls are illegal in the static scope, so to make this part compile we have to wrap it in a function declaration.
// That creates a dynamic scope, in which we can call functions, make variable assignments outside of declarations, and more.
// Infix operators are equivalent to functions, and are thus only allowed in dynamic scopes.
infix_wrapper := fn {

    // An infix operator is a function that takes two arguments and returns one.

    // Example
    a := 2 + 3; // a becomes the int 5

    infix_operator + := fn(int i, float f)->float { return (i)_float + f; };
    // this also implicitly defines the inverse:
    // infix_operator + := fn(float f, int i)->float { return (i)_float + f; }; // the same body, but different function signature
    // the implicit inverse can be overwritten with another operator overload, if a symmetric relation is not wanted.


    // built in operators:
    // infix_operator + : fn(int, int)->int;
    // infix_operator - : fn(int, int)->int;
    // infix_operator * : fn(int, int)->int;
    // infix_operator / : fn(int, int)->int; // truncates the result
    // infix_operator % : fn(int, int)->int;
    // infix_operator < : fn(int, int)->bool;
    // infix_operator > : fn(int, int)->bool;
    // infix_operator <= : fn(int, int)->bool;
    // infix_operator >= : fn(int, int)->bool;
    // infix_operator == : fn(int, int)->bool;
    // infix_operator != : fn(int, int)->bool;

    // infix_operator + : fn(float, float)->float;
    // infix_operator - : fn(float, float)->float;
    // infix_operator * : fn(float, float)->float;
    // infix_operator / : fn(float, float)->float;
    // infix_operator % : fn(float, float)->float; // 5.7 % 1.1 = 0.2
    // infix_operator < : fn(float, float)->bool;
    // infix_operator > : fn(float, float)->bool;
    // infix_operator <= : fn(float, float)->bool;
    // infix_operator >= : fn(float, float)->bool;
    // infix_operator == : fn(float, float)->bool;
    // infix_operator != : fn(float, float)->bool;

    // infix_operator && : fn(bool, bool)->bool; // and
    // infix_operator || : fn(bool, bool)->bool; // or
    // infix_operator == : fn(bool, bool)->bool; // not xor
    // infix_operator != : fn(bool, bool)->bool; // xor

    /*
    infix prio:

    x / %
    + -
    comparison  // (specialfall?: chainade jämförelser, tex. 0<x<1 -> 0<x && x<1)
    &&
    ||
    */

    // These two are equivalent:
    a = 1*2+3*4/5-6+7*8;
    a = ((((1*2)+((3*4)/5))-6)+(7*8));

    // If a operator is encountered, function inferrence will happen in the following order:
    // 1) User defined operator that matches the arguments
    // 2) User defined operator that matches the arguments in the inverse order
    // 3) Built in operator that matches the arguments
    // 4) Built in operator that matches the arguments in the inverse order

};



// TODO: prefix and suffix operators
// prefix_operator ++(int a)->int {return a+1;}
// suffix_operator ++(int a)->int {return a+1;}
// This is hard, because these operators should be able to change the value of the passed variable,
//   and passing a variable by reference is not implemented yet.


