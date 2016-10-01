# Cube specification v0.4


## Table of contents


## Introduction

This is a reference manual for the Cube programming language.

Cube is a general-purpose language designed with systems programming in mind, with heavy focus on run time performance. It is strongly typed. The grammar is compact and regular, allowing for easy analysis by automatic tools such as integrated development environments.

Cube allows for compile time execution of arbitrary code.

Cube has no built in exception handling system, but strives to give well defined behaviour for many edge cases which makes exceptions redundant. Some run time performance is sacrificed in order to accomplish this.




## Comments

Single line comments are started with "//". These end at the next end of line.
Block comments are contained with "/*" and "*/". Block comments can be nested.
Comments cannot start inside a string literal.




## Integer literals

An integer literal is a sequence of digits representing an integer constant. All integer constants must be in the decimal format.

An integer literal can be implicitly casted to any singed or unsigned integer or floating point type, or to the 'flag' type.




## Floaing-point literals

A floating-point literal is a decimal representation of a floating-point constant. It has an integer part, a decimal point, and a fractional part.

A floating point literal can be implicitly casted to any floating point type.




## String literals

A string literal represents a string constant obtained from concatenating a sequence of characters. There are two forms: raw string literals and here-strings.

Raw string literals are a sequence of characters between double quotes, as in "foo". All characters are interpreted as-is except for backslash '\' which is the escape character. Escaped characters that can be used are the following.

    '\\'    literal backslash
    '\"'    literal double quote
    '\n'    newline

Here-strings are begun with the compiler instruction # string along with a delimiter token followed by whitespace. Any following characters are treated as part of the string until the next delimiter token after whitespace.

Some examples of valid strings are the following.

    "This is a raw string literal."
    # string HERE This is a here-string literal. HERE

    # string HERE
        Here strings can span several lines.
        Also contain the delimiter token (HERE) if not preceeded by whitespace.
        All newlines and other formatting will be included in the string literal.
    HERE




## Identifiers

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




## Keywords

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
    in
    by
    operator




## Variables

A variable is a storage location for holding a value. The set of permissible values is determined by the variable's type.

A variable is declared with the following syntax:

    x : int;        // x has type int and is implicitly initialized to 0.
    x : int = 2;    // x has type int and is assigned the value 2.
    x := 2;         // x is assigned the value 2, and the type is implicitly inferred as int, since 2 is a int literal.
    x := foo();     // x is assigned the return value of the function foo(), computed at run time.




## Constants

A constant acts much like a variable but can never change. The value of constants are always computed compile time and takes no storage space in the final run time program.

    x : int : 2;    // x has type int and is assigned the value 2.
    x :: 2;         // x is assigned the value 2, and the type is implicitly inferred as int, since 2 is a int literal.
    x :: foo();     // x is assigned the return value of the function foo(), computed at compile time.




# Types

A type determines the set of values and operations specific to values of that type. Named instances of the boolean, numeric, and string types are predeclared. Composite types - array, struct, pointer, and function types - may be constructed during compile time.




### Booleans

A boolean type represents the set of Boolean truth values denoted by the predeclared constants true and false. The predeclared boolean type is bool.

    b : bool = true;




### Numeric Types

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




### Strings

A string type represents the set of string values. A string value is a (possibly empty) sequence of bytes.

    s : string = "foo";




### Static sequences

A static sequence is an fixed size set of elements of a single type. The size is a part of the type and is determined at compile time.

    s : [5] int = [1, 2, 3, 4, 5];              // the type of the sequence literal is implicitly inferred as int (the type of the first element)
    s : [10] int = [size=10, int: ...];         // all values are implicitly initialized as 0

Static sequences can be accessed with the [] operator. If the index is negative or larger than the size of the sequence, a temporary default intialized value of the corresponding type is returned.




### Dynamic sequences

A dynamic sequence consists of a header containing the current length, capacity and location of the actual data. The actual data is stored on the heap, and it may be relocated if the size of the sequence changes. Dynamic sequence types are marked by that the [] contains the symbol "..".

    s : [..] int = [1, 2, 3, 4, 5];
    s : [..] int = [size=10, int: ...];

Dynamic sequences works very similarly to static sequences. They can be accessed with the [] operator. If the index is negative, a temporary default intialized value of the corresponding type is returned. If the index is larger than the current size, the sequence will insert default initialized values until the necessary size is reached, then the requested value is returned.




### Maps

A map is a set of objects which is indexed with a specific key type. Map types are marked by that the [] contains the type identifier defining the key type.

    s : [int] int = [1->1, 2->2];
    s : [string] int = ["a"->1, "b"->2];




### Structs

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




### Functions

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

If a function is declared as a constant, additional metadata from the initial function literal can also be used in the function call.

    f2 :: fn(a:int=3, b:int=4)->int { return a+2*b; }; // declared as a constant, evaluated compile time
    f2(1, 2); // returns 5.
    f2(b=1, a=2); // returns 4.
    f2(1); // b has the explicit default value 4, so this returns 9.
    f2(b=0); // a has the explicit default value 3, so this returns 3.

The default value of a function is an empty code block.

Values are by default passed to the function by constant references, and thus all in parameters are treated as constants.




### Generic functions

Constant declared functions can also be generic. Generic types are determined at compile time and new versions of the function is automatically generated each time the function is called with a new type. Generic types are marked with '$'.

    f1 :: fn(t : $T) -> T { /*...*/ }; // the type $T is determined by the type of the given input.
    i : int; f : float;
    f1(i); // T is int
    f1(f); // T is float

    f2 :: fn($T : type) -> T { /*...*/ }; // the type $T is determined by a type given as input parameter.
    f2(int); // T is int
    f2(float); // T is float

Even if the generic type is used serveral times in the function declaration, the '$' marker should only be written once. The input parameter marked with the '$' is the one that is responsible for the type inference.

    foo :: fn(t1 : $T, t2 : T) {};
    i : int; f : float;
    // foo(i, f); // gives error: "Type mismatch, expected int but found float for argument 2 in foo(...)";
    // foo(f, i); // gives error: "Type mismatch, expected float but found int for argument 2 in foo(...)";




### Pointers

A pointer type denotes the set of all pointers to variables of a given type, called the base type of the pointer. There are two kinds of pointers: owning and sharing pointers. The default value for both pointer types are null. If a null pointer would be dereferenced, a default initialized temporary value of the base type is returned.

An owning pointer is pointing to and responsible for an object on the heap. When the pointer is destroyed, the object is automatically deallocated.

A sharing pointer is pointing to an already existing object, managed by something else. If a object would be destroyed and then a shared pointer to it would be dereferenced, the dereference would have undefined behaviour.

    p1 : *! int = alloc(2);     // p1 is an owning pointer
    p2 : * int = p1;            // p2 is a sharing pointer which points to the object owning by p1

Returning an owning pointer from a function does not move the allocated object. If the returned variable is assigned to a shared pointer, a temporary anonymous owning pointer is created in that scope instead.

Using an owning pointer as an argument to a function taking a owning pointer in parameter will mark the pointer as destroyed, without moving the actual object. The ownership is simply passed into the function scope instead, and the object is destroyed at the end of that scope.

Sharing pointers and their base type can be used in exactly the same way, and can be substituted for each other. Owning pointers behaves exactly like objects, but they are allocated on the heap instead of the stack.

    T :: struct { data : int; };
    object_1 : T;
    object_2 : T;
    sharing_1 : * T = object_1;
    sharing_2 : * T = object_2;
    owning_1 : *! T; // implicitly allocates memory on the heap
    owning_1 : *! T = ---; // implicitly allocates memory on the heap, but doesn't initalize it

    object_1.data;
    sharing_1.data; // implicitly dereferences the pointer
    owning_1.data; // implicitly dereferences the pointer

    object_2 = object_1; // assigning to object from object -> creates a copy
    sharing_2 = object_1; // assigning to pointer from object -> the pointer now points to the object
    object_2 = sharing_1; // assigning to object from pointer -> creates a copy of the object that the pointer is pointing to
    sharing_2 = sharing_1; // assigning to pointer from pointer -> both pointers now points at the same object

    object_2 = owning_1; // copies the object
    sharing_2 = owning_1; // copes the adress
    owning_2 = object_1; // copies the object
    owning_2 = sharing_1; // copies the object
    owning_2 = owning_1; // copies the object




## Operators

Operators is a terse way of writing function calls. An operator is defined by it preceding type(s), name, and succeeding type(s). Operators can be declared only as constants. The syntax for an operator identifier is the following.

    operator (pre_type1, pre_type2, ...) operator_name (suc_type1, suc_type2, ...)

Either the preceeding type list or the succeeding type list might be empty. An operator with no preceding types is defined as a suffix operator. An operator with no succeeding types is defined as a prefix operator. An operator with both preceding and succeeding types is defined as an infix operator.

Any recogniced non-reserved symbol can be used as operator name. However, they must match the defined format for that specific operator. E.g. '==' is only recognized as an infix operator, and therefore, trying to overload it as a prefix operator will result in a compile error.

Additionally, any identifier can be used as the operator name. This allows for great flexibility.

    operator (int)++             // suffix operator (increment)
    operator !(bool)            // prefix operator (logical NOT)
    operator (bool)and(bool)    // infix operator (logical and)

When calling and operator that has more than one preceding or succeeding types, comma separated parenthesis must be used. If the operator only has one preceding/succeeding type, the paranthesis is optional.

    operator (int)sum(int) :: fn(a: int, b: int)->int { return a+b; };
    total1 := 1 sum 2 sum 3;

    operator (int,int)sum :: fn(a: int, b: int)->int { return a+b; };
    total2 := ((1, 2)sum, 3)sum;

    operator sum(int,int) :: fn(a: int, b: int)->int { return a+b; };
    total3 := sum(1, sum(2, 3));




## Symbols

System defined symbols are the only exclusions of valid variable name characters (excluding the starting character). A symbol can consist of one or more UTF-8 characters. Any non-reserved symbol can be overloaded as an operator.

The following symbols are reserved and cannot be used as operators.

    '(' and ')'
    '[' and ']'
    '{' and '}'
    '->'
    ','
    '.'
    '#'
    ':'
    '='
    '$'
    '---'
    ';'
    '?'
    '''
    '"'
    '`'

The following symbols are recogniced as valid operators. They are listed with their respective priority. Higher priority always goes before lower priority. If two operators have the same priority, they are evaluated from left to right. Multiple prefix operators are evaluated from right to left.

    symbol          priority        note
    '[]' (infix)    1000            indexing operator / gets a value - defined as infix but used as suffix (NOTE: see problem below)
    '[]=' (infix)   1000            indexing operator / sets a value - defined as infix but used as suffix (NOTE: see problem below)
    '()' (reserved) 1000            reserved; only listed here to give priority context. Function operator.
    '.' (reserved)  1000            reserved; only listed here to give priority context. Getter operator.
    '++' (suffix)   1000            increment
    '--' (suffix)   1000            decrement

    '+' (prefix)    900             unary plus
    '-' (prefix)    900             unary minus
    '*' (prefix)    900             pointer dereference
    '!' (prefix)    900             logical NOT
    '++' (prefix)   900             increment
    '--' (prefix)   900             decrement
    '@' (prefix)    900
    '&' (prefix)    900
    '~' (prefix)    900

    '^' (infix)     800             to the power of

    '*' (infix)     700             multiplication
    '/' (infix)     700             division
    '%' (infix)     700             modulo (remainder)
    '/%' (infix)    700

    '+' (infix)     600             addition
    '-' (infix)     600             subtraction

    '<<' (infix)    550
    '>>' (infix)    550
    '<<<' (infix)   550
    '>>>' (infix)   550

    '<' (infix)     500             less than
    '>' (infix)     500             greater than
    '<=' (infix)    500             less than or equal to
    '>=' (infix)    500             greater than or equal to
    '>~' (infix)    500
    '<~' (infix)    500

    '==' (infix)    400             equal to
    '!=' (infix)    400             not equal to

In addition, all operators defined with an identifier as operator name have the following priority:

    symbol          priority        note
    id (suffix)     950             operator using identifier (not a symbol)
    id (prefix)     850             operator using identifier (not a symbol)
    id (infix)      0               operator using identifier (not a symbol)

Reading a new expression, operators are examined as follows.

1. Check if the first token is a prefix operator. If so, read a new expression and build a prefix operator node.
2. If not, read non-operator expression (literal or identifier)
3. Check if the next token is a suffix operator. If so, build a suffix operator node.
4. If not, check if the next token is an infix operator. If so, read a new expression and build an infix operator node.




### Assignment operators

After an infix operator is used, it can also be used for terse assignment together with the '=' symbol. The requirements for this is the following.
* The operator has to take exactly one preceding type argument
* That preceding type has to be the same as the return type from the operator.

Example:

    a : int;

    // Assuming operator(int)+(int) : fn(int, int)->int,
    //   these are all equivalent:
    a = a + 2;
    a + = 2;
    a += 2;



## Expressions

Expressions




<!--
# TODO

    <<
    >>
    <<<
    >>>
    &
    |
    &&
    ||
    ^
    ~
    >~
    <~
    /%
    @



PROBLEM WITH [] overloading / pointer syntax:

seq : [4]int;
seq[2] = 2; // the position 2 should now have the value 2
i := seq[3]; // i should be the int 0



















TODO: decide if this is allowed
    // '()'                        function operator



## Threads(TODO)

Functions may be called asynchronously using the keyword "async".

    foo();              // runs foo() directly and waits for it to finish before continue the program flow.
    async foo();        // starts a new thread and runs foo() there. After the thread is created this statement is complete - it doesn't wait for foo() to complete.

Asynchronous function calls can not return anything. Any in parameters will be passed by deep copy (instead of the normal const reference).

*** TODO: channels, or some other way to allow different threads to synchronize or communicate



















TODO:

Statements
    declaration
    assignment
    function with side effects


Expressions


Operators


type any


Maps

    s : [string] int = ["a"->1, "b"->2];
    s : [string] int = [string->int: ];



for i, v in [1, 2, 3] {}
for k, v in ["a"->1, "b"->2] {}
for i, v in 1..2 {}

blank identifier, _



    static_seq : [5] int;       // [] contains the number of elements in the sequence (compile time calculated integer value)
    dynamic_seq : [..] int;     // [] contains the symbol ".."
    map : [string] int;         // [] contains the key type





true, false: build in constant identifiers of type Bool








 and has explicit support for concurrent programming.
Programs are constructed from packages, whose properties allow efficient management of dependencies. The existing implementations use a traditional compile/link model to generate executable binaries.



The length of a string s (its size in bytes) can be discovered using the built-in function len. The length is a compile-time constant if the string is a constant. A string's bytes can be accessed by integer indices 0 through len(s)-1.










Go channels:

    messages := make(chan string)           // create the channel
    go func() { messages <- "ping" }()      // put a message in the channel in a different thread (blocks the thread until the message is read)
    msg := <- messages;                     // receive the message in this thread (blocks the thread until a message is received)


Buffered channels:

    messages := mage(chan string, 2)        // create the channel, buffering up to 2 values
    messages <- "buffered"                  // put 2 values in the buffer
    messages <- "channel"                   // if a 3rd value would be put in here, it would block the thread until a message was read
    msg1 := <- messages                     // receive 2 values from the buffer
    msg2 := <- messages                     // if a 3rd value would be read here, it would block the thread until a message was sent

Receive only channels (chan<-):

    func ping(pings chan<- stringm, msg string) { pings <- msg }

Read only channels (<-chan):

    func pong(pings <-chan string, pongs chan<- string) { msg:=<-pings; pongs<-msg }

Waiting for several channels simultaneously with select (also showing timeouts):

    select {
        case msg1 := <-channel1: foo1()                 // if msg1 was received first, do foo1()
        case msg2 := <-channel2: foo2()                 // if msg2 was received first, do foo2()
        case <-time.After(time.Second*2): foo3()        // if none was received before 2 seconds, do foo3()
    }

Non-blocking operations with select default:

    select {
        case msg1 := <-channel1: foo1()                 // if msg1 has a message, do foo1()
        case msg2 := <-channel2: foo2()                 // if msg2 has a message, do foo2()
        default: foo3();                                // if none of them have a message waiting, do foo3()
    }

-->