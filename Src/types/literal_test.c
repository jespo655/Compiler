

#ifdef __CPLUSPLUS
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"

typedef struct { uint32_t type; void* v_ptr; } any;
bool bt = true;
bool bf = false;
bool b1 = 1;
bool b0 = 0;

uint8_t cs[5];

typedef struct
{
    uint8_t cs[5];
    uint16_t b;
} css;

css csss[2];


int main() {
    any a;
    a = (any){ 32, NULL };
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
}

#ifdef __CPLUSPLUS
}
#endif





