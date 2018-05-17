
#ifndef _COMPILED_C_H
#define _COMPILED_C_H

#ifdef __CPLUSPLUS
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"

// basic types
typedef uint32_t _cb_type;
typedef struct { _cb_type type; void* v_ptr; } _cb_any;
typedef bool _cb_bool;
typedef int8_t _cb_i8;
typedef int16_t _cb_i16;
typedef int32_t _cb_i32;
typedef int64_t _cb_i64;
typedef uint8_t _cb_u8;
typedef uint16_t _cb_u16;
typedef uint32_t _cb_u32;
typedef uint64_t _cb_u64;
typedef float _cb_f32;
typedef double _cb_f64;
typedef int64_t _cb_int;
typedef uint64_t _cb_uint;
typedef double _cb_float;
typedef uint8_t _cb_flag;
typedef struct { _cb_i64 r_start; _cb_i64 r_end; } _cb_i_range;
typedef struct { _cb_f64 r_start; _cb_f64 r_end; } _cb_f_range;
typedef char* _cb_string;

// complex types
typedef void(*_cb_type_22)(_cb_int*);
typedef void(*_cb_type_23)();

// function declarations
void _cb_fn_1(_cb_int i, _cb_int* r);


#ifdef __CPLUSPLUS
}
#endif

#endif
