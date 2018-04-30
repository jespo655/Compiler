

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

int main() {
    any a;
    a = (any){ 32, NULL };
    printf("hw\n");
    printf("a.type: = %u\n", a.type);
    printf("bools: %u %u %u %u\n", bt, bf, b1, b0);
}

#ifdef __CPLUSPLUS
}
#endif





