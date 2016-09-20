[[Cube specification v0.4]]


[[Table of contents]]


[[Introduction]]

This is a reference manual for the Cube programming language.

Cube is a general-purpose language designed with systems programming in mind, with heavy focus on run time performance. It is strongly typed. The grammar is compact and regular, allowing for easy analysis by automatic tools such as integrated development environments.

Cube allows for compile time execution of arbitrary code.

Cube has no built in exception handling system, but strives to give well defined behaviour for many edge cases which makes exceptions redundant. Some run time performance is sacrificed in order to accomplish this.

[[Comments]]

Single line comments are started with "//". These end at the next end of line.
Block comments are contained with "/*" and "*/". Block comments can be nested.
Comments cannot start inside a string literal.

[[Integer literals]]

An integer literal is a sequence of digits representing an integer constant. All integer constants must be in the decimal format.

An integer literal can be implicitly casted to any singed or unsigned integer or floating point type, or to the 'flag' type.

[[Floaing-point literals]]

A floating-point literal is a decimal representation of a floating-point constant. It has an integer part, a decimal point, and a fractional part.

A floating point literal can be implicitly casted to any floating point type.

[[String literals]]

A string literal represents a string constant obtained from concatenating a sequence of characters. There are two forms: raw string literals and here-strings.

Raw string literals are a sequence of characters between double quotes, as in "foo". All characters are interpreted as-is except for backslash '\' which is the escape character. Escaped characters that can be used are the following.

    '\\'    literal backslash
    '\"'    literal double quote
    '\n'    newline

Here-strings are begun with the compiler instruction #string along with a delimiter token followed by whitespace. Any following characters are treated as part of the string until the next delimiter token after whitespace.

Some examples of valid strings are the following.

    "This is a raw string literal."
    #string HERE This is a here-string literal. HERE

    #string HERE
        Here strings can span several lines.
        Also contain the delimiter token (HERE) if not preceeded by whitespace.
        All newlines and other formatting will be included in the string literal.
    HERE

[[Identifiers]]

An identifier is a name for some variable in the program. Identifiers must follow the following format:
It must start with a letter in [a-z, A-Z].
It must not contain any symbols (see the Symbols section) except for underscore "_".
It may end with "!", "?" or any combination of these.

Example of valid identifiers:
    several_words
    numb3rs
    question?
    imperative!
    what?!!??

Example of invalid identifiers:
    1_starts_with_number
    _starts_with_underscore
    contains?symbol

[[Keywords]]

The following keywords are reserved and may not be used as identifiers.

    if
    elsif
    else
    fn
    return
    defer
    for
    while
    struct

[[Variables]]

A variable is a storage location for holding a value. The set of permissible values is determined by the variable's type.

A variable is declared with the following syntax:

    x : int;        // x has type int and is implicitly initialized to 0.
    x : int = 2;    // x has type int and is assigned the value 2.
    x := 2;         // x is assigned the value 2, and the type is implicitly inferred as int, since 2 is a int literal.
    x := foo();     // x is assigned the return value of the function foo(), computed at run time.

[[Constants]]

A constant acts much like a variable but can never change. The value of constants are always computed compile time and takes no storage space in the final run time program.

    x : int : 2;    // x has type int and is assigned the value 2.
    x :: 2;         // x is assigned the value 2, and the type is implicitly inferred as int, since 2 is a int literal.
    x :: foo();     // x is assigned the return value of the function foo(), computed at compile time.

[[Types]]

A type determines the set of values and operations specific to values of that type. Named instances of the boolean, numeric, and string types are predeclared. Composite types - array, struct, pointer, and function types - may be constructed during compile time.

[[Booleans]]

A boolean type represents the set of Boolean truth values denoted by the predeclared constants true and false. The predeclared boolean type is bool.

    b : bool = true;

[[Numeric Types]]

A numeric type represents sets of integer or floating-point values. The predeclared architecture-independent numeric types are:

    u8          the set of all unsigned  8-bit integers (0 to 255)
    u16         the set of all unsigned 16-bit integers (0 to 65535)
    u32         the set of all unsigned 32-bit integers (0 to 4294967295)
    u64         the set of all unsigned 64-bit integers (0 to 18446744073709551615)
    uint        the same as u64, but can also be implicitly casted to any other numeric type.

    i8          the set of all signed  8-bit integers (-128 to 127)
    i16         the set of all signed 16-bit integers (-32768 to 32767)
    i32         the set of all signed 32-bit integers (-2147483648 to 2147483647)
    i64         the set of all signed 64-bit integers (-9223372036854775808 to 9223372036854775807)
    int         the same as i64, but can also be implicitly casted to any other numeric type.

    f32         the set of all IEEE-754 32-bit floating-point numbers
    f64         the set of all IEEE-754 64-bit floating-point numbers
    float       the same as f64, but can also be implicitly casted to any other floating point type.

    byte        alias for u8

The value of an n-bit integer is n bits wide and represented using two's complement arithmetic.

[[Strings]]

A string type represents the set of string values. A string value is a (possibly empty) sequence of bytes.

    s : string = "foo";

[[Static sequences]]

A static sequence is an fixed size set of elements of a single type. The size is a part of the type and is determined at compile time.

    s : [5] int = [1, 2, 3, 4, 5];              // the type of the sequence literal is implicitly inferred as int (the type of the first element)
    s : [10] int = [size=10, int: ...];         // all values are implicitly initialized as 0

Static sequences can be accessed with the [] operator. If the index is negative or larger than the size of the sequence, a temporary default intialized value of the corresponding type is returned.

[[Dynamic sequences]]

A dynamic sequence consists of a header containing the current length, capacity and location of the actual data. The actual data is stored on the heap, and it may be relocated if the size of the sequence changes.

    s : [..] int = [1, 2, 3, 4, 5];
    s : [..] int = [size=10, int: ...];

Dynamic sequences works very similarly to static sequences. They can be accessed with the [] operator. If the index is negative, a temporary default intialized value of the corresponding type is returned. If the index is larger than the current size, the sequence will insert default initialized values until the necessary size is reached, then the requested value is returned.

[[Structs]]

A struct type is a sequence of named elements, called fields, each of which has a name and a type and possibly a default value. Each struct type, identified by the "struct" keyword, is unique.

    S1 : type = struct {};          // an empty struct type
    S2 : type = struct {            // a struct type with 3 fields
        a : int;
        b : float;
        c : string = "Hello";
    }

Struct instances can then be created just like for any type.

    s1 : S1;                        // s1 is a default initialized struct of type S1.
    s2 : S2 = make_S2();            // s2 is a S2 returned by the function make_S2.

[[Functions]]

A function type is defined as a set of in (argument) and out (return) types. A function variable are essentially a pointer to the code to be executed. The syntax is the following.

    fn(in_type_1, in_type_2, ...) -> (out_type_1, out_type_2, ...)      // A function type with a set of in and out types. Each set can be empty.
    fn(in_type_1, in_type_2, ...) -> out_type_1                         // A function type with a set of in types and one out type
    fn(in_type_1, in_type_2, ...)                                       // A function type with a set of in types but no out type

A function literal is defined as a block of code that begins with instantiations of the in and out types. All in parameters has to be named. Out parameters can be either named or anonymous. Named parameters can have explicit default values, evaluated compile time when the function literal is created. Some examples of valid function literals are the following:

    fn(a: int, b:int)-> c:int { /* function code, using variables a, b and c */ }
    fn(a: int, b:int)-> int { /* ... */ }
    fn(a: int = 1, b:int = 2)-> c:int = 3 { /* ... */ }

Function names are treated just like any other variable or constant. A function variable has the size of a pointer.

Functions can be called using the () operator.

    f1 := fn(a:int, b:int)->int { return a+2*b; }; // declared as a variable / function pointer, its value can change at run time
    f1(1, 2); // returns 5

If the function are declared as a constant, additional metadata from the initial function literal can also be used in the function call.

    f2 :: fn(a:int=3, b:int=4)->int { return a+2*b; }; // declared as a constant, evaluated compile time
    f2(1, 2); // returns 5.
    f2(b=1, a=2); // returns 4.
    f2(1); // b has the explicit default value 4, so this returns 9.
    f2(b=0); // a has the explicit default value 3, so this returns 3.

The default value of a function is an empty code block.

[[Generic functions]]

*** TODO

*** Uses function literal meta data, so only possible for constant functions

[[Pointers]]

A pointer type denotes the set of all pointers to variables of a given type, called the base type of the pointer. There are two kinds of pointers: owning and sharing pointers. The default value for both pointer types are null. If a null pointer would be dereferenced, a default initialized temporary value of the base type is returned.

An owning pointer is pointing to and responsible for an object on the heap. When the pointer is destroyed, the object is automatically deallocated.

A sharing pointer is pointing to an already existing object, managed by something else. If a object would be destroyed and then a shared pointer to it would be dereferenced, the dereference would have undefined behaviour.

    p1 : *! int = alloc(2);     // p1 is an owning pointer
    p2 : * int = p1;            // p2 is a sharing pointer which points to the object owning by p1

Returning an owning pointer from a function does not move the allocated object. If the returned variable is assigned to a shared pointer, a temporary anonymous owning pointer is created in that scope instead.

Using an owning pointer as an argument to a function taking a owning pointer in parameter will mark the pointer as destroyed, without moving the actual object. The ownership is simply passed into the function scope instead, and the object is destroyed at the end of that scope.


















TODO:

Statements
    declaration
    assignment
    function with side effects


Expressions


Operators









true, false: build in constant identifiers of type Bool








 and has explicit support for concurrent programming.
Programs are constructed from packages, whose properties allow efficient management of dependencies. The existing implementations use a traditional compile/link model to generate executable binaries.



The length of a string s (its size in bytes) can be discovered using the built-in function len. The length is a compile-time constant if the string is a constant. A string's bytes can be accessed by integer indices 0 through len(s)-1.