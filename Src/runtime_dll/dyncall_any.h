#pragma once

/*
Extension for dll.h for use with Seq<Any>
*/

#include "../types/cb_any.h"
#include "../utilities/sequence.h"
#include "dll.h"

namespace dll {

    void call_fn_any(void* fn_ptr, Seq<Any> args);

}
