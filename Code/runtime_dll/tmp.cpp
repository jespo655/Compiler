#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

void test() { printf("CUSTOM DLL FUNCTION\n"); }
void test_str(char const* str) { printf("CUSTOM DLL FUNCTION: %s\n", str); }
}

