// #include "dyncall/include/dyncall.h"
#include "dll.h"

#include <iostream>
#include <string>

using namespace dll;

// DCCallVM* create_vm() {
//     DCCallVM* vm = dcNewCallVM(4096);
//     dcMode(vm, DC_CALL_C_DEFAULT);
//     return vm;
// }

void hw(char* s) {
    printf("hello %s\n", s);
}


std::string fn_str = "void fn(char* s, int i) { printf(\"hello from dll: %s %d\\n\", s, i); }";
std::string fn2_str = "void fn2() { printf(\"hello from dll with no arguments\\n\"); }";


int main()
{
    // DCCallVM* vm = create_vm();
    auto dll = compile_dll({create_src({"stdio.h"}, {fn_str, fn2_str})});
    std::string fn_name = "fn";
    std::string fn_name2 = "fn2";
    auto fn = load_fn<void(*)(char*,int)>(dll, fn_name);
    auto fn2 = load_fn(dll, fn_name2);

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

    // call_fn_tmp(fn);
    call_fn((void*)fn, s, 3);
    call_fn((void*)fn2);

    printf("success!\n");
}

