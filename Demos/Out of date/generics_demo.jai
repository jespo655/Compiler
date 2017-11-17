

// a generic function has one or more types in the in_paremeter list prefixed with $
// That argument will be used to infer the type.

f1 : fn(a : $T) -> T{ return a; };
f2 : fn(a : T, b : $T) -> T { return a+b; };


generic_wrapper : fn {

    a : int;
    b := f1(a); // will generate f1(int)->int, so b will become a int

    c : float;
    d := f1(c); // will generate f1(float)->float, so d will become a float

    e : string;
    f := f2(e,e); // will generate f2(string,string)->string, which would require the infix_operator + (string,string)->string to be defined
                  // if infix_operator + (string,string)->string is not defined, then this will generate a compile error

}