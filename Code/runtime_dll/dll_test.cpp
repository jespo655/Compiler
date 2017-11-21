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


std::vector<std::string> lines = {
    "void fn(char* s, int i) { printf(\"hello from dll: %s %d\\n\", s, i); }",
    // "void fn(char* s, int i) { printf(\"hello from dll: %s %d\\n\", s, i); }",
    "void fn2() { printf(\"hello from dll with no arguments\\n\"); }",
    "int set_i(int arg, int* i) { printf(\"setting i to %d\\n\", arg); *i = arg; return arg; }",
};


int main()
{
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

