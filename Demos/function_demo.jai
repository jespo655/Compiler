
// Functions
// /*Syntax:*/ funcion_name := fn(in_parameters)->(out_parameters) { body };

// Function calls are illegal in the static scope, so to make this part compile we have to wrap it in a function declaration.
// That creates a dynamic scope, in which we can call functions, make variable assignments outside of declarations, and more.
fn_wrapper := fn() -> () {

    f1 := fn(a : int, b : int) -> (int) { return a+b; };

    // A function can hanve more than one return value
    f2 := fn() -> (int, float) { return 2, 2.0; };
    r2a, r2b := f2(); // r2a becomes the int 2, r2b becomes the float 2.0

    // LHS and RHS must contain the same number of values, but a returned value can be ignored by the dump variable "_".
    r2c, _ := f2(); // r2c becomes the int 2
    _, r2d := f2(); // r2d becomes the float 2.0

    // If only one return value is used, the out_parameter parenthesis is unnecessary.
    f3 := fn(i: int) -> int { return i+3; };

    // If a function returns nothing, then the arrow can also be skipped.
    f4 := fn() {};

    // If a function takes no in_parameters, then the first parenthesis can be skipped.
    f5 := fn -> int { return 5; };

    // The return values can be named, and then used as variables in the function.
    // At the end of the function, the variables' values are returned.
    f6 := fn -> (r:int) { r = 2; } // returns 2
    f7 := fn -> (r:int) { r = 2; return 3; } // return statement overwrites previous value of r, so this returns 3


    // More advanced functions:
    // Accessing named parameters out of order
    f8 := fn -> (c:int, d:int) { return d=2; c=1; }; // returns (1,2)
    f9 := fn(a:int, b:int) -> (int, int) { return a, b; };
    r9a, r9b := f9(b=1,a=2); // returns (2,1)

    // functions returning functions
    f10 := fn -> fn { return f4; };

    // The following are equivalent to the definition of f10 above, should also work
    // f10 := fn() -> (fn) { return f4; };
    // f10 := fn -> (fn()->()) { return f4; };

    // Recursive functions
    fib_recursive := fn(n:int,a:int,b:int)->int
    {
        if (n<=0) { return a; }
        return fib_recursive(n-1,b,a+b);
    };
    fib1 = fib_recursive(5,0,1);

    // Default values in in parameters
    fib := fn(n:int,a:int=0,b:int=1)->int
    {
        if (n<=0) { return a; }
        return fib(n-1,b,a+b);
    };
    fib2 = fib(5,0,1);
    fib3 = fib(5,0);
    fib4 = fib(5);
    fib5 = fib(5,b=1);

};


// Notes:
// Function overloading is not allowed: A function is treated the same as any
//   other variable, and thus cannot have more than one type and one value.