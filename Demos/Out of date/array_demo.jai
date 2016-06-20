// array_demo.jai


a : [3] int = (1,2,3); // fixed size 3
a2 : [] int = (1,2,3,4,5); // fixed size gets inferred to 5
b : [..] int = (1,2,3,4,5); // dynamic size

c := (1,2,3); // defaults to a fixed size 3. All items in the array are of same type -> c becomes an int array


f1 := fn -> int { return 1; };
f2 := fn -> (int) { return 1; }
f3 := fn -> (int,int,int) { return 1,2,3; };
f4 := fn -> (int, float, string) { return 1, 2.0, "3"; };

arr_wrapper := fn {

    b[9] = 10; // since b is a dynamic array, the size is increased from 5 to 10, padded with 0s
    b_100 = b[100]; // since b now only have 10 places, this returns 0, which is the default value of int
    // a[10] = 10; // runtime error since we know that a has a fixed size of 3
    // a_10 = a[10]; // runtime error since we know that a has a fixed size of 3
    // b[-1] = 1; // runtime error: array index cannot be negative

    d := f1(); // d becomes the int 1
    e := f2(); // f2 is equivalent to f2, so e also becomes the int 1
    fa, fb, fc := f3(); // fa, fb, fc becomes 1, 2 and 3
    g := f3(); // g becomes an array containing the elements 1,2,3. Since all of them are ints, g beomces an int array.

    // the "any" type:
    h : [] any = (1,2.0,"3"); // can store elements of any type
    i := f4(); // i becomes an any array of size 3, containing the elements (1,2.0,"3")

}

