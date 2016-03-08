



// (int i = i2 = i3), float f, f = 2, 4.5, f; // OK genom parser!


// funktion: inte klart - rhs start tillåter inte "fn" keyword
// foo = fn(int a, float b) -> int, int
// {
//     return a, b;
// };


// a, b = foo(2,3.5); // ska inte vara ok i static scope: inga funktioner tillåtna


// struct: inte klart - rhs start tillåter inte "struct" keyword
// s = struct {
//     int a;
//     int b;
// };


// scope: inte klart - rhs start tillåter inte scope identifier [ eller {
// scope = {};





