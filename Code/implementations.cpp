
#include "abstx/abstx.h"
#include "abstx/function.h"
#include "abstx/identifier.h"
#include "utilities/assert.h"
#include "utilities/unique_id.h"
#include <string>

/*
    TEMPORARY FILE

    The function implementations in this file cannot exist in header files for some reason
    Preferrably, they should be modified so they work in their respective header file

    If that's not possible, they should be moved to other implementation files where they fit in better
*/






// problem: cannot be declared static in a header file - that
// will make separate instances of "id" in different files
int get_unique_id() {
    static int id=0;
    ASSERT(id >= 0); // if id is negative then the int has looped around. More than INT_MAX unique identifiers should never be needed.
    return id++;
}

std::string get_unique_id_str() {
    return std::to_string(get_unique_id());
}










