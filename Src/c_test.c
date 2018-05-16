

#ifdef __CPLUSPLUS
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"



typedef void(*cb_fn)();

void _cb_fn_4436() { printf("foo\n"); }

// cb_fn foo = _cb_fn_4436;


void fn_test() {

    void _cb_fn_4437() { printf("foo\n"); }

    cb_fn foo = _cb_fn_4436;
    foo();

}


// typedef struct {
//     void* v_ptr;
//     uint32_t type;
// } any;
// bool bt = true;
// bool bf = false;
// bool b1 = 1;
// bool b0 = 0;

// uint8_t cs[5];

// typedef struct
// {
//     uint16_t b;
//     uint8_t cs[5];
// } css;

// typedef struct
// {

// } empty;

// css csss[2];


// void struct_test()
// {
//     any a;
//     a.v_ptr = NULL;
//     // a = (any){ NULL, 32 };
//     // a = (any){ 0, NULL };
//     printf("hw\n");
//     printf("a.type: = %u\n", a.type);
//     printf("bools: %u %u %u %u\n", bt, bf, b1, b0);
//     any a2;
//     a2 = a;
//     printf("a2.type: = %u\n", a2.type);
//     printf("sizeof(5a): %d\n", sizeof(cs));
//     printf("sizeof(5b): %d\n", sizeof(css));
//     printf("sizeof(5c): %d\n", sizeof(csss));
//     css x;
//     printf("b pos: %d\n", (char*)&x.b-(char*)&x);
//     printf("cs pos: %d\n", (char*)&x.cs[0]-(char*)&x);

//     printf("sizeof(empty): %d\n", sizeof(empty));
// }

void for_test()
{
    int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    int _it;
    for (size_t _it_123 = 0; _it = a[_it_123], _it_123 < 10; ++_it_123)
    {
        // int _it = a[_it_123];
        printf("#%u = %d\n", _it_123, _it);
    }
}

// typedef uint32_t _cb_type;
// typedef struct { _cb_type type; void* v_ptr; } _cb_any;
// typedef bool _cb_bool;
// typedef int8_t _cb_i8;
// typedef int16_t _cb_i16;
// typedef int32_t _cb_i32;
// typedef int64_t _cb_i64;
// typedef uint8_t _cb_u8;
// typedef uint16_t _cb_u16;
// typedef uint32_t _cb_u32;
// typedef uint64_t _cb_u64;
// typedef float _cb_f32;
// typedef double _cb_f64;
// typedef int64_t _cb_int;
// typedef uint64_t _cb_uint;
// typedef double _cb_float;
// typedef uint8_t _cb_flag;
// typedef struct { _cb_f64 r_start; _cb_f64 r_end; } _cb_range;
// typedef char* _cb_string;
// typedef void(*_cb_type_21)(_cb_i8, _cb_range const*, _cb_float*, _cb_range*);
// void foo(_cb_i8 in1, _cb_range const* in2, _cb_float* out1, _cb_range* out2) {
//     printf("hello world!\n");
// }

int main() {
    fn_test();

    // _cb_type_21 f = foo;
    // _cb_u8 u8;
    // _cb_range range;
    // _cb_float fl;
    // f(u8, &range, &fl, &range);

    // struct_test();
    // for_test();
}

#ifdef __CPLUSPLUS
}
#endif





