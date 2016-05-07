

// Function calls are illegal in the static scope, so to make this part compile we have to wrap it in a function Declaration
// That creates a dynamic scope, in which we can call functions, make variable assignments outside of declarations, and more.
clause_wrapper := fn() -> () {

    // if-elsif-else-then clause
    // /*Syntax:*/ if condition { body } elsif condition { body } else { body } then { body }
    a := 2;
    if a<1 {
        // do stuff if a<1
    } elsif a<4 {
        // do stuff if not(a<1) but a<4
    } elsif a<10 {
        // do stuff if not(a<1) and not(a<4) but a<10
    } else {
        // do stuff if not(a<1) and not(a<4) and not(a<10)
    } then {
        // do stuff if else wasn't done
    }


    // while loop
    // /*Syntax:*/ while condition { body }
    a = 10;
    while a > 0 {
        a = a - 1;
    }


    // for loop
    // /*Syntax:*/ for iterator_name in range by step { body }
    // A second local variable, called it_index, is created, counting the number of times the loop has been run. (starting with 0)

    // The iterator will be local to the body scope, so it cannot clash with earlier defined variables
    for a in 1 .. 6 by 2 {} // will enter body with a = 1, 3, 5. it_index will have the values 0, 1, 2

    // Step can be skipped
    for a in 1 .. 6 {} // will enter body with a = 1, 2, 3, 4, 5, 6. it_index will have the values 0, 1, 2, 3, 4, 5

    // Iterator name can be skipped. Default name is "it"
    for a in 1 .. 6 {} // will enter body with it = 1, 2, 3, 4, 5, 6. it_index will have the values 0, 1, 2, 3, 4, 5

    // For more ranges, see the array demo.



};