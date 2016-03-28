

// 2(); // ASSERT failed, borde ge error

dump : int;

fib := fn(n:int,a:int,b:int)->int
{
    if (n<=0) { return a; }
    return fib(n-1,b,a+b);
};

// a:type= struct{a:int;b,c:float,float;};
// c:a;
// (i1=i2=i3) := 2;

// aaaaaaaaa := 2; // variabel definierad med det första tokenet får ibland ett konstigt namn (É eller é).
//                 // datan i id->token->token skrivs över av någon anledning
// q2 := a.b.c.d.e.f.g;

// a, b, c := 2, "asd", 1.0; // ok

// d := d; // ger note men inget error!

// (i = i2 = i3), f : int, float = 2, 4.5; // OK genom parser!


// b := d();



// scope: ok!
// scope = {
//     s = {};
//     int a = 2;
//     d += int f; // error - bad rhs
//     f // error - no statement
//     f; // error - no statement
//     ] // error - expected "}" before "]
//     (); // error - no statement;
//     a = (); // error - unexpected parens in a context where no functions are allowed

//     arr = a[2]; // TODO: arrays
// };





// funktion: inte klart - rhs start tillåter inte "fn" keyword
// foo := fn(int a, float b) -> int, int
// {
//     return a, b;
// };


// a, b = foo(2,3.5); // ska inte vara ok i static scope: inga funktioner tillåtna


// struct: inte klart - rhs start tillåter inte "struct" keyword
// s = struct {
//     int a;
//     int b;
// };






