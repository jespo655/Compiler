// #define DLL_TEST
#ifdef DLL_TEST


// #include "dyncall/include/dyncall.h"
#include "dll.h"

#include <iostream>
#include <string>
#include <vector>

using namespace dll;

// DCCallVM* create_vm() {
//     DCCallVM* vm = dcNewCallVM(4096);
//     dcMode(vm, DC_CALL_C_DEFAULT);
//     return vm;
// }

void hw(char* s) {
    printf("hello %s\n", s);
}


int i = 0;



struct S {
    int a;
    int b;
    int c;
    int d;
};

std::vector<std::string> struct_lines = {
    "typedef struct S {",
    "uint8_t a;",
    "uint64_t b;",
    "uint16_t c;",
    "int d;",
    "} S;",
    "void sum_s(S* s) { printf(\"a=%u, b=%u, c=%u, d=%u, size=%u\\n\", (uint8_t)s->a, (uint8_t)s->b, (uint8_t)s->c, (uint8_t)s->d, sizeof(S));",
    // "printf(\"pos: a=%u, b=%u, c=%u, d=%u\\n\", (uint8_t)((uint8_t*)&s->a-(uint8_t*)s), (uint8_t)((uint8_t*)&s->b-(uint8_t*)s), (uint8_t)((uint8_t*)&s->c-(uint8_t*)s), (uint8_t)((uint8_t*)&s->d-(uint8_t*)s), sizeof(S));",
    "}",
};


void struct_test() {
    auto dll = compile_dll({create_src({"stdint.h", "stdio.h"}, struct_lines)});
    std::string fn_name = "sum_s";
    auto fn = load_fn(dll, fn_name);
    uint8_t s_a[32];
    for (int i = 0; i < sizeof(s_a); ++i) s_a[i] = i;
    int i = 0;
    call_fn((void*)fn, &s_a, &i);

    printf("i is %d\n", i);
};



// slutsatser C alignment:
// u8 är inte alignade
// u16 är alignade till 16bit
// u32 är alignade till 32bit
// u64 är alignade till 64bit






std::vector<std::string> lines = {
    "void fn(char* s, int i) { printf(\"hello from dll: %s %d\\n\", s, i); }",
    // "void fn(char* s, int i) { printf(\"hello from dll: %s %d\\n\", s, i); }",
    "void fn2() { printf(\"hello from dll with no arguments\\n\"); }",
    "int set_i(int arg, int* i) { printf(\"setting i to %d\\n\", arg); *i = arg; return arg; }",
};


int main()
{
    struct_test();
    return 0;


    // DCCallVM* vm = create_vm();
    auto dll = compile_dll({create_src({"stdio.h"}, lines)});
    std::string fn_name = "fn";
    std::string fn_name2 = "fn2";
    auto fn = load_fn<void(*)(char*,int)>(dll, fn_name);
    auto fn2 = load_fn(dll, fn_name2);
    auto set_i = load_fn(dll, std::string("set_i"));

    remove_temp_files();

    char s[] = "world";
    // dcReset(vm);
    // dcArgPointer(vm, s);
    // dcArgInt(vm, 1);
    // dcCallVoid(vm, (void*)fn);

    // dcReset(vm);
    // dcArgPointer(vm, s);
    // dcArgInt(vm, 2);
    // dcCallVoid(vm, (void*)fn);

    printf("i is %d\n", i);

    // call_fn_tmp(fn);
    call_fn((void*)fn, s, 3);
    call_fn((void*)fn2);
    call_fn((void*)set_i, 1337, &i);

    printf("i is %d\n", i);

    printf("success!\n");
}


#endif