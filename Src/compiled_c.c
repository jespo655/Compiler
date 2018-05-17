
#ifdef __CPLUSPLUS
extern "C" {
#endif

#include "compiled_c.h"
#include "stdio.h"

void _cb_fn_1(_cb_int i, _cb_int* r) {
    printf("hello world #%d!\n", i);
    *r = 2;
}

void main() {
    _cb_int _cb_tmp_3;
    _cb_fn_1(2ULL, &_cb_tmp_3);
    _cb_int i = _cb_tmp_3;
    printf("%d\n", i);
}

#ifdef __CPLUSPLUS
}
#endif
