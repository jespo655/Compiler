#include "literal.h"
#include "type.h"
#include <memory>

/*
This file creates objects for default values and built in types,
and stores them on the heap through a std::shared_ptr<>.

That way all reference counting will work as expected, and each
type and default literal are only stored once.

In some cases, there are no specific default literal for a certain
type. Those cases are solved elsewhere.
*/


/************************** int **************************/

#include "int.h"

const std::shared_ptr<const Literal> DEFAULT_INT_LITERAL{new Literal_int()};

const std::shared_ptr<const Type> BUILT_IN_TYPE_i8{new Type_i8()};
const std::shared_ptr<const Type> BUILT_IN_TYPE_i16{new Type_i16()};
const std::shared_ptr<const Type> BUILT_IN_TYPE_i32{new Type_i32()};
const std::shared_ptr<const Type> BUILT_IN_TYPE_i64{new Type_i64()};


std::shared_ptr<const Literal> Type_i8::get_default_value() const
{
    return DEFAULT_INT_LITERAL;
}

std::shared_ptr<const Literal> Type_i16::get_default_value() const
{
    return DEFAULT_INT_LITERAL;
}

std::shared_ptr<const Literal> Type_i32::get_default_value() const
{
    return DEFAULT_INT_LITERAL;
}

std::shared_ptr<const Literal> Type_i64::get_default_value() const
{
    return DEFAULT_INT_LITERAL;
}


std::shared_ptr<const Type> Literal_int::get_type()
{
    return BUILT_IN_TYPE_i64;
}



/************************** uint **************************/

#include "int.h"

const std::shared_ptr<const Literal> DEFAULT_UINT_LITERAL{new Literal_uint()};

const std::shared_ptr<const Type> BUILT_IN_TYPE_u8{new Type_u8()};
const std::shared_ptr<const Type> BUILT_IN_TYPE_u16{new Type_u16()};
const std::shared_ptr<const Type> BUILT_IN_TYPE_u32{new Type_u32()};
const std::shared_ptr<const Type> BUILT_IN_TYPE_u64{new Type_u64()};

std::shared_ptr<const Literal> Type_u8::get_default_value() const
{
    return DEFAULT_UINT_LITERAL;
}

std::shared_ptr<const Literal> Type_u16::get_default_value() const
{
    return DEFAULT_UINT_LITERAL;
}

std::shared_ptr<const Literal> Type_u32::get_default_value() const
{
    return DEFAULT_UINT_LITERAL;
}

std::shared_ptr<const Literal> Type_u64::get_default_value() const
{
    return DEFAULT_UINT_LITERAL;
}


std::shared_ptr<const Type> Literal_uint::get_type()
{
    return BUILT_IN_TYPE_u64;
}


/************************** float **************************/

#include "float.h"

const std::shared_ptr<const Literal> DEFAULT_FLOAT_LITERAL{new Literal_float()};

const std::shared_ptr<const Type> BUILT_IN_TYPE_f32{new Type_f32()};
const std::shared_ptr<const Type> BUILT_IN_TYPE_f64{new Type_f64()};

std::shared_ptr<const Literal> Type_f32::get_default_value() const
{
    return DEFAULT_FLOAT_LITERAL;
}

std::shared_ptr<const Literal> Type_f64::get_default_value() const
{
    return DEFAULT_FLOAT_LITERAL;
}

std::shared_ptr<const Type> Literal_float::get_type()
{
    return BUILT_IN_TYPE_f64;
}


/************************** type **************************/

const std::shared_ptr<const Type> BUILT_IN_TYPE_type{new Type_type()};

std::shared_ptr<const Type> Type::get_type()
{
    return BUILT_IN_TYPE_type;
}



/************************** seq **************************/

#include "seq.h"
#include "scope.h"

std::shared_ptr<const Type> Literal_seq::get_type()
{

    std::shared_ptr<Type_seq> t{new Type_seq()};
    t->type = type;
    t->size = size;
    auto t_id = t->toS();

    // check if the type already exists
    auto scope = parent_scope();
    ASSERT(scope != nullptr);
    auto tp = scope->get_type(t_id);

    if (tp != nullptr) return tp; // if so, return the already existing pointer
    scope->types[t_id] = t; // else add it to the list of types
    return t;
}

// The default value is generated dynamically depending on the sequence type.



/************************** str **************************/

#include "str.h"

const std::shared_ptr<const Literal> DEFAULT_STR_LITERAL{new Literal_str()};

const std::shared_ptr<const Type> BUILT_IN_TYPE_str{new Type_str()};

std::shared_ptr<const Literal> Type_str::get_default_value() const
{
    return DEFAULT_STR_LITERAL;
}

std::shared_ptr<const Type> Literal_str::get_type()
{
    return BUILT_IN_TYPE_str;
}



/************************** bool **************************/

#include "bool.h"

const std::shared_ptr<const Literal> DEFAULT_BOOL_LITERAL{new Literal_bool()};

const std::shared_ptr<const Type> BUILT_IN_TYPE_bool{new Type_bool()};

std::shared_ptr<const Literal> Type_bool::get_default_value() const
{
    return DEFAULT_BOOL_LITERAL;
}

std::shared_ptr<const Type> Literal_bool::get_type()
{
    return BUILT_IN_TYPE_bool;
}




/************************** range **************************/

#include "range.h"

const std::shared_ptr<const Literal> DEFAULT_RANGE_LITERAL{new Literal_range()};

const std::shared_ptr<const Type> BUILT_IN_TYPE_range{new Type_range()};

std::shared_ptr<const Literal> Type_range::get_default_value() const
{
    return DEFAULT_RANGE_LITERAL;
}

std::shared_ptr<const Type> Literal_range::get_type()
{
    return BUILT_IN_TYPE_range;
}




/************************** struct **************************/

#include "struct.h"

// no struct literals are allowed.





/************************** scope **************************/

#include "scope.h"

const std::shared_ptr<const Literal> DEFAULT_DYNAMIC_SCOPE_LITERAL{new Literal_scope(true)};
const std::shared_ptr<const Literal> DEFAULT_STATIC_SCOPE_LITERAL{new Literal_scope(false)};

const std::shared_ptr<const Type> BUILT_IN_TYPE_dynamic_scope{new Type_scope(true)};
const std::shared_ptr<const Type> BUILT_IN_TYPE_static_scope{new Type_scope(false)};

std::shared_ptr<const Literal> Type_scope::get_default_value() const
{
    if (dynamic) return DEFAULT_DYNAMIC_SCOPE_LITERAL;
    else         return DEFAULT_STATIC_SCOPE_LITERAL;
}

std::shared_ptr<const Type> Literal_scope::get_type()
{
    if (dynamic) return BUILT_IN_TYPE_dynamic_scope;
    else         return BUILT_IN_TYPE_static_scope;
}