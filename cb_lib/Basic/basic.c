#include <stdlib.h> // malloc, free
#include <string.h> // memset

// Types

typedef char cb_i8;
typedef short cb_i16;
typedef int cb_i32;
typedef long long cb_i64;

typedef unsigned char cb_u8;
typedef unsigned short cb_u16;
typedef unsigned int cb_u32;
typedef unsigned long long cb_u64;

typedef float cb_f32;
typedef double cb_f64;

typedef cb_i64 cb_int;
typedef cb_u64 cb_uint;
typedef cb_f64 cb_float;
typedef cb_u64 cb_type;

typedef cb_u8 cb_byte;
typedef cb_byte cb_bool;
const cb_bool true = 1;
const cb_bool false = 0;

typedef struct {
    cb_u64 length;
    cb_byte* data;
} Seq;

typedef char* String; // \0-terminated sequence of UTF-8 characters

typedef struct {
    cb_f64 start;
    cb_f64 end;
} Range;

typedef struct {
    cb_type type;
    void* value;
} Any;

// struct Map {}; // TODO

// This list should be extended in the compiled program with all used special types
#define TYPE_i8 0
#define TYPE_i16 1
#define TYPE_i32 2
#define TYPE_i64 3
#define TYPE_u8 4
#define TYPE_u16 5
#define TYPE_u32 6
#define TYPE_u64 7
#define TYPE_f32 8
#define TYPE_f64 9
#define TYPE_string 10
#define TYPE_Range 11
#define TYPE_Any 12

// TYPE_Seq
// TYPE_Ptr














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

cb_u64 closest_higher_power_of_2(cb_u64 v)
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

cb_u64 dynamic_seq_capacity(Seq* Seq)
{
    cb_u64 v = Seq->length<16?16:Seq->length;
    return closest_higher_power_of_2(v);
}

void resize_seq(Seq* Seq, cb_u64 data_size, cb_u64 length) {
    if (length < Seq->length) {
        // TODO - requires proper deletion of structs (with owning pointers and that kind of stuff)
    } else if (length > dynamic_seq_capacity(Seq)) {
        cb_u64 new_capacity = closest_higher_power_of_2(length);
        void* new_data = malloc(new_capacity);
        memcpy(new_data, Seq->data, Seq->length);
        memset(new_data+Seq->length*data_size, 0, (new_capacity-Seq->length)*data_size); // this should be default constructors for that type
        free(Seq->data);
        Seq->data = (cb_byte*)new_data;
        Seq->length = length;
    }
}

void static_seq_index(Seq* Seq, cb_u64 data_size, cb_uint index, cb_byte** __retval) {
    // assert(index < length);
    *__retval = Seq->data + index*data_size;
}

void dynamic_seq_index(Seq* Seq, cb_u64 data_size, cb_uint index, cb_byte** __retval) {
    if (Seq->data == NULL) {
        cb_u64 new_capacity = closest_higher_power_of_2(index+1);
        Seq->data = (cb_byte*)malloc(new_capacity);
        // assert(Seq->data != NULL);
        Seq->length = index+1;
        memset(Seq->data, 0, new_capacity*data_size); // this should be default constructors for that type
    } else if (index >= Seq->length && index >= dynamic_seq_capacity(Seq)) {
        cb_u64 new_capacity = closest_higher_power_of_2(index+1);
        void* new_data = malloc(new_capacity);
        // assert(new_data != NULL);
        memcpy(new_data, Seq->data, Seq->length);
        memset(new_data+Seq->length*data_size, 0, (new_capacity-Seq->length)*data_size); // this should be default constructors for that type
        free(Seq->data);
        Seq->data = (cb_byte*)new_data;
        Seq->length = index+1;
    }
    *__retval = Seq->data + index * data_size;
}





/** Strings **/

void string_concat(String a, String b, String* __retval) {
    cb_u64 a_len = strlen(a);
    cb_u64 b_len = strlen(b);
    free(*__retval);
    *__retval = (String)malloc(a_len+b_len+1);
    memcpy(*__retval, a, a_len);
    memcpy((*__retval)+a_len, b, b_len);
    (*__retval)[a_len+b_len] = '\0';
}

void lt_string_string(String a, String b, cb_bool* __retval) { *__retval = strcmp(a, b) < 0; }
void eq_string_string(String a, String b, cb_bool* __retval) { *__retval = strcmp(a, b) == 0; }




/** Memory allocation **/

void alloc(cb_u64 size, void** ptr) {
    free(*ptr);
    ptr = malloc(size);
    memset(ptr, 0, size);
}




typedef void(*Destructor)(void*);

void destroy_string(void* ptr) {
    // all strings are heap allocated
    free(*(String*)ptr);
}

void destroy_sharing_ptr(void* ptr) {
    // do nothing
}

void destroy_owning_ptr(void* ptr) {
    // free the object
    free(*(void**)ptr);
}

void destroy_seq(Seq* Seq, Destructor destr) {
    int i;
    for (i = 0; i < Seq->length; ++i) {
        destr(&Seq->data[i]);
    }
}
void destroy_dynamic_seq(Seq* Seq, Destructor destr) {
    destroy_seq(Seq, destr);
    free(Seq->data);
}



void destroy_any(Any* any) {
    switch(any->type) {
        case TYPE_string: {
            destroy_seq(((Seq*)any->value), destroy_string);
        }
    }
}





int main() {}











