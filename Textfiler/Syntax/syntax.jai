

// Basic:


// Deklaration:

a : int; // a är av typen int. Får alltid default value av typen, i det här fallet 0
a : int = 5; // a är av typen int, och får värdet 5
a := 5; // typen kan inferras från värdet som tilldelas. 5 är en int-literal, så a blir en int.
a, b, c := 1, 2, 3; // flera saker kan deklareras/tilldelas på samma rad. Endast ett =-tecken är tillåtet.
a, b, c : int, int, int = 1, 2, 3; // om typer definieras måste antalet stämma överens.


// Dump-variabel, "_":

a, b, _ : int, _, int, = 1, 2, 3; // dump-variabeln kan ersättas med det som passar. Dump-variabeln kan tilldelas vilket värde av vilken typ som helst,
                                  // som ett sätt att säga att värdet inte är intressant. Om _ används som typ sker en type inferance från tilldelningsvärdet.



// Basic types:

// inbyggt i språket:
// i8       8 bit signed integer
// i16      16 bit signed integer
// i32      32 bit signed integer
// i64      64 bit signed integer
// int      32 or 64 bit signed integer. Either platform specific, or always 64 bit (TBD).

// u8       8 bit unsigned integer
// u16      16 bit unsigned integer
// u32      32 bit unsigned integer
// u64      64 bit unsigned integer
// uint     32 or 64 bit unsigned integer. Either platform specific, or always 64 bit (TBD).

// f32      32 bit floating point number
// f64      64 bit floating point number
// float    32 or 64 bit floating point number. Either platform specific, or always 64 bit (TBD).

// bool     boolean, stored as u8 where 0 = false and other values = true
// flag     works as a offset, stored in u8
// string   A string of characters. Exact implementation TBD.
// type     The type of other types. "type" also has the type type.

// sequence / array: (sequences exists of any type, here using int as an example)
// int[]    a fixed size sequence of int. Allocated on the stack. Always bounds checked unless specified.
// int[..]  a dynamic size sequence of int. Allocated on the heap. Always bounds checked unless specified.

// set (maybe) (sets exists of any type, here using int as an example)
// int{}    a set of int. Allocated on the heap in a tree structure. Sets are slow


// Function types:

// fn(int, int) -> (int, int) // a fn that takes 2 ints and returns 2 ints
// fn(int) -> int // with only one return value, the return parenthesis can be skipped
// fn -> int // with no arguments, the argument parenthesis can be skipped
// fn(int) // with no return value, the return arrow can be skipped

// Struct types:

// T = struct { a:int; b:int=2; c:=3; }; // struct{} creates a new type, but it can't be used directly but has to be assigned to a type identifier.


// Literals:

a := 1; // int
a := 1.0; // float
a := ""; // string
a := int; // type
a := [1, 2, 3]; // int[] of size 3
a := [int: 1, 2, 3]; // int[] of size 3
a := [int, count=5: 1, 2, 3]; // int[] of size 5. Remaining 2 values are defaulted to 0. If count=X specifies a too low number to keep all given members, the remaining members will be skipped.
// there is no literal for dynamic seqs, but a fix seq literal can be assigned to a specified dynamic seq variable.
// a := [] // illegal, since the type of the seq cannot be inferred.
a := {1, 2, 3}; // int{}
a := {int: 1, 2, 3} // int{}
// a := {}; // illegal, since the type of the set cannot be inferred.
a := fn {}; // empty function literal. Has to have function body enclosure {}
a := fn(a:int, b:int)->int { return a+b; }; // fn(int,int)->int. Arguments has to be named.
a := fn(a:int, b:int)->r:int { r=a+b; }; // fn(int,int)->int. Return parameters can be named.
a := fn(a:int=2) {}; // default values are allowed. After the first argument with default value, all remaining also has to have default values.











// flags

flag f0 = 0;
flag f1 = 1;
flag f2 = 2;
flag f3 = 3;
flag f4 = 4;
flag f5 = 5;
flag f5 = 6;
flag f5 = 7;

i8 bits = f1+f2; // 00000110
i8 bits = f1+f1; // 00000001
i8 bits = (f1+f1) - f1; // 00000000
i8 bits = f7; // 10000000

// TODO: hur kollar man om en flagga är satt med snygg syntax?
// bits.is_set(f0)
// bits == f0





// operator overloading

infix_operator + = fn(a:T1, b:T2)->r:T3 {}; // implicitly defines the reverse unless specified








/*
function: type is serialized and used as identifier

map from serialized fn name+type to abs_stx_node
if default values: the same fn is mapped several times with a different number of arguments

fn_node has a list of in and out parameters
each one has:
    * name (could be automatically generated for return values)
    * type
    * default value (can be inferred from the type) (only literals allowed)

The type of the parameters can be used to infer the function type
The type of the in parameters are used for type checking


when looking up functions, check for perfect matches first
then generic functions
then implicit cast one thing at the time, check for close matches first
*/






// generic functions

fn($T, T) -> T; // the type T is determined of the first type sent in
// when the function is called, a new version of the function is generated,
// with all occurences of the type T replaced with the new type.
// That string is then compiled into a new function.


// added: map from fn name -> generic function, that is checked only
// if the regular map doesn't find a result.
// If a matchable generic fn is found, a new regular function is created
// and inserted in the regular fn map.



















