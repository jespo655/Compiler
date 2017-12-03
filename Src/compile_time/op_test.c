#include "built_in_operators.h"
#include "stdio.h"
// #include "stdlib.h"
#include "string.h"


struct T{ int i; };


// void test(T& t) {
//     t.i = 2;
// }


void print_a(int* a, int c)
{
    int first = 1;
    for (int i = 0; i < c; i++) {
        if (!first) printf(", ");
        printf("%d", a[i]);
        first = 0;
    }
    printf("\n");
}



int main()
{
    int_least8_t i;


    // TODO: förstå memset, sätt in memset i value::alloc

    // för strings, seq, array gör ett

    // memset:
    // memset(void* dst, byte value, int count)



    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = i;
    }

    print_a(arr, 10);
    memset(arr, 0x10001, 2);
    print_a(arr, 10);


    // int_least8_t a = 1;
    // int_least8_t b = 2;
    // int_least8_t c;
    // _infix_operator_plus_i8_i8(a, b, &i);
    // _infix_operator_minus_i8_i8(b, a, &i);
    // printf("%d\n",i);
}