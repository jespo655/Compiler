

#ifdef __CPLUSPLUS
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"

typedef struct {
    void* v_ptr;
    uint32_t type;
} any;
bool bt = true;
bool bf = false;
bool b1 = 1;
bool b0 = 0;

uint8_t cs[5];

typedef struct
{
    uint16_t b;
    uint8_t cs[5];
} css;

typedef struct
{

} empty;

css csss[2];


void struct_test()
{
    any a;
    a.v_ptr = NULL;
    // a = (any){ NULL, 32 };
    // a = (any){ 0, NULL };
    printf("hw\n");
    printf("a.type: = %u\n", a.type);
    printf("bools: %u %u %u %u\n", bt, bf, b1, b0);
    any a2;
    a2 = a;
    printf("a2.type: = %u\n", a2.type);
    printf("sizeof(5a): %d\n", sizeof(cs));
    printf("sizeof(5b): %d\n", sizeof(css));
    printf("sizeof(5c): %d\n", sizeof(csss));
    css x;
    printf("b pos: %d\n", (char*)&x.b-(char*)&x);
    printf("cs pos: %d\n", (char*)&x.cs[0]-(char*)&x);

    printf("sizeof(empty): %d\n", sizeof(empty));
}

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


int main() {
    // struct_test();
    for_test();
}

#ifdef __CPLUSPLUS
}
#endif





