#pragma once


#include "../utilities/assert.h"
#include "cb_any.h"
#include "cb_function.h"
#include "cb_pointer.h"
#include "cb_primitives.h"
#include "cb_range.h"
#include "cb_seq.h"
#include "cb_string.h"
#include "cb_struct.h"
#include "cb_type.h"


/*
This is a list of built in types
Each built in type is represented by a c++ struct
An instance of a type can have a value of that type

Things that is supplied here:

* The field 'static CB_Type type', which is
    an unique type identifier. It does not take any space,
    and is always constexpr in compiled code.

* Implicit cast operators (if applicable)

* std::string toS() function



Built in functions that refer to types:
* sizeof(Type t)

*/

template<typename T, typename S=std::string(T::*)() const, S=T::toS>
std::ostream& operator<<(std::ostream& os, const T& t) { return os << t.toS(); }
