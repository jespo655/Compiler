#include <stdlib.h> // malloc, free
#include <string.h> // memset

// Types

typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef float f32;
typedef double f64;

typedef i64 cb_int;
typedef u64 cb_uint;
typedef f64 cb_float;

typedef u8 byte;
typedef byte cb_bool;
#define true 1
#define false 0

typedef struct {
    u64 length;
    byte* data;
} Seq;


typedef char* String; // \0-terminated sequence of UTF-8 characters

// struct Map {}; // TODO





/** Operators **/

void mult_float_float(cb_float a, cb_float b, cb_float* __retval) { *__retval = a*b; }
void mult_float_int(cb_float a, cb_int b, cb_float* __retval) { *__retval = a*b; }
void mult_float_uint(cb_float a, cb_uint b, cb_float* __retval) { *__retval = a*b; }
void mult_int_int(cb_int a, cb_int b, cb_int* __retval) { *__retval = a*b; }
void mult_int_uint(cb_int a, cb_uint b, cb_int* __retval) { *__retval = a*b; }

void plus_float_float(cb_float a, cb_float b, cb_float* __retval) { *__retval = a+b; }
void plus_float_int(cb_float a, cb_int b, cb_float* __retval) { *__retval = a+b; }
void plus_float_uint(cb_float a, cb_uint b, cb_float* __retval) { *__retval = a+b; }
void plus_int_int(cb_int a, cb_int b, cb_int* __retval) { *__retval = a+b; }
void plus_int_uint(cb_int a, cb_uint b, cb_int* __retval) { *__retval = a+b; }

void divide_float_float(cb_float a, cb_float b, cb_float* __retval) { *__retval = a/b; }
void divide_float_int(cb_float a, cb_int b, cb_float* __retval) { *__retval = a/b; }
void divide_float_uint(cb_float a, cb_uint b, cb_float* __retval) { *__retval = a/b; }
void divide_int_int(cb_int a, cb_int b, cb_int* __retval) { *__retval = a/b; }
void divide_int_uint(cb_int a, cb_uint b, cb_int* __retval) { *__retval = a/b; }

void minus_float_float(cb_float a, cb_float b, cb_float* __retval) { *__retval = a-b; }
void minus_float_int(cb_float a, cb_int b, cb_float* __retval) { *__retval = a-b; }
void minus_float_uint(cb_float a, cb_uint b, cb_float* __retval) { *__retval = a-b; }
void minus_int_int(cb_int a, cb_int b, cb_int* __retval) { *__retval = a-b; }
void minus_int_uint(cb_int a, cb_uint b, cb_int* __retval) { *__retval = a-b; }

void mod_float_float(cb_float a, cb_float b, cb_float* __retval) { *__retval = a - b*(int)(a/b); }
void mod_float_int(cb_float a, cb_int b, cb_float* __retval) { *__retval = a - b*(int)(a/b); }
void mod_float_uint(cb_float a, cb_uint b, cb_float* __retval) { *__retval = a - b*(int)(a/b); }
void mod_int_int(cb_int a, cb_int b, cb_int* __retval) { *__retval = a%b; }
void mod_int_uint(cb_int a, cb_uint b, cb_int* __retval) { *__retval = a%b; }

void lt_float_float(cb_float a, cb_float b, cb_float* __retval) { *__retval = a<b; }
void lt_float_int(cb_float a, cb_int b, cb_bool* __retval) { *__retval = a<b; }
void lt_float_uint(cb_float a, cb_uint b, cb_bool* __retval) { *__retval = a<b; }
void lt_int_int(cb_int a, cb_int b, cb_bool* __retval) { *__retval = a<b; }
void lt_int_uint(cb_int a, cb_uint b, cb_bool* __retval) { *__retval = a<b; }

void eq_float_float(cb_float a, cb_float b, cb_float* __retval) { *__retval = a==b; }
void eq_float_int(cb_float a, cb_int b, cb_bool* __retval) { *__retval = a==b; }
void eq_float_uint(cb_float a, cb_uint b, cb_bool* __retval) { *__retval = a==b; }
void eq_int_int(cb_int a, cb_int b, cb_bool* __retval) { *__retval = a==b; }
void eq_int_uint(cb_int a, cb_uint b, cb_bool* __retval) { *__retval = a==b; }
void eq_bool_bool(cb_bool a, cb_bool b, cb_bool* __retval) { *__retval = a==b; }

void not_bool(cb_bool b, cb_bool* __retval) { *__retval = !b; }

void and_bool_bool(cb_bool a, cb_bool b, cb_bool* __retval) { *__retval = a && b; }
void or_bool_bool(cb_bool a, cb_bool b, cb_bool* __retval) { *__retval = a || b; }
void xor_bool_bool(cb_bool a, cb_bool b, cb_bool* __retval) { *__retval = a ^ b; }



/** Sequences **/

u64 closest_higher_power_of_2(u64 v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    v++;
    return v;
}

u64 dynamic_seq_capacity(Seq* seq)
{
    u64 v = seq->length<16?16:seq->length;
    return closest_higher_power_of_2(v);
}

void resize_seq(Seq* seq, u64 data_size, u64 length) {
    if (length < seq->length) {
        // TODO - requires proper deletion of structs (with owning pointers and that kind of stuff)
    } else if (length > dynamic_seq_capacity(seq)) {
        u64 new_capacity = closest_higher_power_of_2(length);
        void* new_data = malloc(new_capacity);
        memcpy(new_data, seq->data, seq->length);
        memset(new_data+seq->length*data_size, 0, (new_capacity-seq->length)*data_size); // this should be default constructors for that type
        free(seq->data);
        seq->data = (byte*)new_data;
        seq->length = length;
    }
}

void static_seq_index(Seq* seq, u64 data_size, cb_uint index, byte** __retval) {
    // assert(index < length);
    *__retval = seq->data + index*data_size;
}

void dynamic_seq_index(Seq* seq, u64 data_size, cb_uint index, byte** __retval) {
    if (seq->data == NULL) {
        u64 new_capacity = closest_higher_power_of_2(index+1);
        seq->data = (byte*)malloc(new_capacity);
        // assert(seq->data != NULL);
        seq->length = index+1;
        memset(seq->data, 0, new_capacity*data_size); // this should be default constructors for that type
    } else if (index >= seq->length && index >= dynamic_seq_capacity(seq)) {
        u64 new_capacity = closest_higher_power_of_2(index+1);
        void* new_data = malloc(new_capacity);
        // assert(new_data != NULL);
        memcpy(new_data, seq->data, seq->length);
        memset(new_data+seq->length*data_size, 0, (new_capacity-seq->length)*data_size); // this should be default constructors for that type
        free(seq->data);
        seq->data = (byte*)new_data;
        seq->length = index+1;
    }
    *__retval = seq->data + index * data_size;
}





/** Strings **/

void string_concat(String a, String b, String* __retval) {
    u64 a_len = strlen(a);
    u64 b_len = strlen(b);
    free(__retval);
    __retval = malloc(a_len+b_len+1);
    memcpy(__retval, a, a_len);
    memcpy(__retval+a_len, b, b_len);
    __retval[a_len+b_len] = '\0';
}

void lt_string_string(String a, String b, cb_bool* __retval) { *__retval = strcmp(a, b) < 0; }
void eq_string_string(String a, String b, cb_bool* __retval) { *__retval = strcmp(a, b) == 0; }





// int main() {}











