
#include "dll.h"
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream> // debug

using namespace dll;

std::string base_filename = "_tmp_";


#ifdef __WIN32

#include <windows.h>
std::string del_cmd = "del";

// returns null if dll can't be found
dll_handle dll::load_dll(std::string filename)
{
    return LoadLibrary(filename.c_str());
}

bool free_dll(dll_handle dll)
{
    return FreeLibrary(dll);
}

void* dll::load_fn_ptr(dll_handle dll, std::string fn_name)
{
    if (dll) return (void*)GetProcAddress(dll, fn_name.c_str());
    return nullptr;
}

#else
std::string del_cmd = "rm -rf";
#endif









int dll_counter = 0;
dll_handle dll::compile_dll(std::vector<std::string> src_files)
{
    std::ostringstream dll{};
    std::ostringstream cmd{};
    dll << base_filename << ++dll_counter << ".dll";
    cmd << "gcc -Shared -o " << dll.str();
    for (std::string& file_name : src_files) cmd << " " << file_name;
    system(cmd.str().c_str());
    return load_dll(dll.str());
}

// extern "C" wrapper to prevent c++ name mangling
std::string extern_c_header = "\n#ifdef __cplusplus\nextern \"C\" {\n#endif\n";
std::string extern_c_footer = "\n#ifdef __cplusplus\n}\n#endif\n";

int src_counter = 0;
std::string dll::create_src(std::vector<std::string> includes, std::vector<std::string> lines)
{
    std::ostringstream src{};
    src << base_filename << ++src_counter << ".c";
    std::ofstream ofs{src.str()};
    for (std::string& include : includes) ofs << "#include \"" << include << "\"" << std::endl;
    ofs << extern_c_header << std::endl;
    for (std::string& line : lines) ofs << line << std::endl;
    ofs << extern_c_footer << std::endl;
    ofs.close();
    return src.str();
}

void dll::remove_temp_files()
{
    std::ostringstream oss{};
    oss << del_cmd << " " << base_filename << "*";
    #ifdef __WIN32
    oss << " 2>nul"; // suppress error messages
    #endif
    auto r = system(oss.str().c_str());
}






namespace dll {
namespace dll_internal {

    #include "dyncall/include/dyncall.h"

    DCCallVM* vm;
    const DCsize VM_SIZE = 4096;

    DCCallVM* reset_vm() {
        if (vm == nullptr) {
            vm = dcNewCallVM(VM_SIZE);
            dcMode(vm, DC_CALL_C_DEFAULT);
        }
        dcReset(vm);
        return vm;
    }

    // template<typename ret_t> ret_t _call_fn_internal(void* fn_ptr) {  return dcCallPointer(vm, fn_ptr); } // assume that it's a pointer. We should get compile error if it's not
    template<> void _call_fn_internal<void>(void* fn_ptr) { dcCallVoid(vm, fn_ptr); }
    template<> bool _call_fn_internal<bool>(void* fn_ptr) { return dcCallBool(vm, fn_ptr); }
    template<> char _call_fn_internal<char>(void* fn_ptr) { return dcCallChar(vm, fn_ptr); }
    template<> short _call_fn_internal<short>(void* fn_ptr) { return dcCallShort(vm, fn_ptr); }
    template<> int _call_fn_internal<int>(void* fn_ptr) { return dcCallInt(vm, fn_ptr); }
    template<> long _call_fn_internal<long>(void* fn_ptr) { return dcCallLong(vm, fn_ptr); }
    template<> long long _call_fn_internal<long long>(void* fn_ptr) { return dcCallLongLong(vm, fn_ptr); }
    template<> float _call_fn_internal<float>(void* fn_ptr) { return dcCallFloat(vm, fn_ptr); }
    template<> double _call_fn_internal<double>(void* fn_ptr) { return dcCallDouble(vm, fn_ptr); }

    // template<typename T> void _push_fn_arg(T t) { dcArgPointer(vm, t); } // assume it's a pointer
    template<> void _push_fn_arg<bool>(bool value) { dcArgBool(vm, value); }
    template<> void _push_fn_arg<char>(char value) { dcArgChar(vm, value); }
    template<> void _push_fn_arg<short>(short value) { dcArgShort(vm, value); }
    template<> void _push_fn_arg<int>(int value) { dcArgInt(vm, value); }
    template<> void _push_fn_arg<long>(long value) { dcArgLong(vm, value); }
    template<> void _push_fn_arg<long long>(long long value) { dcArgLongLong(vm, value); }
    template<> void _push_fn_arg<float>(float value) { dcArgFloat(vm, value); }
    template<> void _push_fn_arg<double>(double value) { dcArgDouble(vm, value); }

} // namespace dll_internal
} // namespace dll
