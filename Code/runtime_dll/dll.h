#pragma once

#include <string>
#include <vector>

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

// removes temp files. Dlls that is currently will not be removed.
void clean_up();

