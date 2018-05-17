
#ifdef __CPLUSPLUS
extern "C" {
#endif

#include "compiled_c.h"
#include "stdio.h"

// @TODO: it's better to define function pointers like this
// Probably good for all constant values
#define hw _cb_fn_1
#define _cb_main _cb_fn_2
// _cb_type_22 hw = _cb_fn_1;
// _cb_type_23 _cb_main = _cb_fn_2;

void _cb_fn_1(_cb_int i, _cb_int* r) {
    printf("hello world #%d!\n", i);
    *r = 2;
}

void _cb_fn_2() {
    _cb_int _cb_tmp_3;
    hw(2ULL, &_cb_tmp_3);
    _cb_int i = _cb_tmp_3;
    printf("%d\n", i);
}

void main() {
    _cb_main();
}

#ifdef __CPLUSPLUS
}
#endif
