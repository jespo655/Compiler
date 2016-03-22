


a, b := 2, 3;


// (int i = i2 = i3), float f, f = 2, 4.5, f; // OK genom parser!

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






