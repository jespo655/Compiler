# Compiler

A compiler project, heavily inspired by [JAI](https://github.com/BSVino/JaiPrimer/blob/master/JaiPrimer.md).

The primary goal with the project is educational - I want to learn how compilers function. If JAI gets popular I could see the project evolve into a fully functional JAI compiler sometime in the future, but until then I'm happy with experimenting and coming up with my own syntax where I think it fits.


Current timeline of events:

1. Finish specification, including everything except multithreading support.
2. Go through all existing code, see that it works according to the specification. The biggest thing to add is proper functions, especially functions using metadata.
3. Assert that it is possible to build AST "by hand". Write test cases that cover everything in the specification.
4. Write compile time execution code. Run all test cases rom 3. through this.
5. Write backend to compile to C. Run all test cases from 3. through this.
6. Rewrite lexer. A lot of the code can be reused from old versions.
7. Rewrite parser. This should probably be done from scratch, using flowcharts and diagrams to find the best way from the start. If possible, add multithreading support to the compiler itself.
8. Write Cube code covering everything in the specification. Compile this code and run it in both run and compile time.










