#ifndef type_checker_h
#define type_checker_h

#include "parser.h"

/*
Previously in parser:
    For each declaration in a static scope, the identifier was added to the scope.

Next step: Go through each statement and verify that all identifiers can be found.
If dynamic scope -> for each declaration, add the identifier to the scope.
    The order is important there, so we can't add everything beforehand.

If we find an identifier with unknown type -> add dependencies to that specific identifier.
If we resolve the type of a specific identifier -> resolve statements depending on that identifier.

Later: if we fully resolve a #run statement -> run that function!
*/


// bool resolve_evaluated_variable()



#endif