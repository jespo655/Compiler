#pragma once

/*
Minimal interface for dll handling
To call loaded functions, you either need to know the function signature at compile time,
    or use the dyncall library to push the arguments on the stack manually.
*/

#include <string>
#include <vector>

namespace dll {

#ifdef __WIN32
#include <windows.h>
typedef HMODULE dll_handle;
#else
#error dll not implemented on current OS
#endif

// returns nullptr if dll failed to load
dll_handle load_dll(std::string filename);
dll_handle compile_dll(std::vector<std::string> src_files); // requires gcc in path
std::string create_src(std::vector<std::string> includes, std::vector<std::string> lines);

// returns true if successful
bool free_dll(dll_handle dll);

// returns nullptr if fn failed to load
// the function signature has to be correct for the function to work properly.
template<typename fn_t = void(*)()>
fn_t load_fn(dll_handle dll, std::string fn_name);

// returns nullptr if fn failed to load
void* load_fn_ptr(dll_handle dll, std::string fn_name);

// removes temp files. Dlls that is currently will not be removed.
void remove_temp_files();

// internal namespace for internal template functions that we don't want exposed
namespace dll_internal {

    #include "dyncall/include/dyncall.h"

    extern DCCallVM* vm;
    extern const DCsize VM_SIZE;

    template<typename ret_t> ret_t _call_fn_internal(void* fn_ptr) { return dcCallPointer(vm, fn_ptr); } // assume that it's a pointer. We should get compile error if it's not
    template<> void _call_fn_internal<void>(void* fn_ptr);
    template<> bool _call_fn_internal<bool>(void* fn_ptr);
    template<> char _call_fn_internal<char>(void* fn_ptr);
    template<> short _call_fn_internal<short>(void* fn_ptr);
    template<> int _call_fn_internal<int>(void* fn_ptr);
    template<> long _call_fn_internal<long>(void* fn_ptr);
    template<> long long _call_fn_internal<long long>(void* fn_ptr);
    template<> float _call_fn_internal<float>(void* fn_ptr);
    template<> double _call_fn_internal<double>(void* fn_ptr);

    template<typename T> void _push_fn_arg(T t) { dcArgPointer(vm, t); } // assume it's a pointer
    template<> void _push_fn_arg<bool>(bool value);
    template<> void _push_fn_arg<char>(char value);
    template<> void _push_fn_arg<short>(short value);
    template<> void _push_fn_arg<int>(int value);
    template<> void _push_fn_arg<long>(long value);
    template<> void _push_fn_arg<long long>(long long value);
    template<> void _push_fn_arg<float>(float value);
    template<> void _push_fn_arg<double>(double value);

    template<typename ret_t, typename arg_t, typename... arg_ts>
    ret_t _call_fn_internal(void* fn_ptr, arg_t arg, arg_ts... args) {
        _push_fn_arg<arg_t>(arg);
        return _call_fn_internal<ret_t, arg_ts...>(fn_ptr, args...);
    }

    template<typename ret_t=void, typename... arg_ts>
    ret_t _call_fn(void* fn_ptr, arg_ts... args) {
        if (vm == nullptr) {
            vm = dcNewCallVM(VM_SIZE);
            dcMode(vm, DC_CALL_C_DEFAULT);
        }
        dcReset(vm);
        return _call_fn_internal<ret_t, arg_ts...>(fn_ptr, args...);
    }

} // namespace dll_internal

// call a function with some arguments, using dyncall as a backend
template<typename ret_t=void, typename... arg_ts>
ret_t call_fn(void* fn_ptr, arg_ts... args) {
    dll_internal::_call_fn(fn_ptr, args...);
}



} // namespace dll
