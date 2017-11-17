
#ifdef __WIN32

#include "dll.h"
#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream> // debug

std::string base_filename = "_tmp_";
std::string del_cmd = "del";

// returns null if dll can't be found
dll_handle load_dll(std::string filename)
{
    return LoadLibrary(filename.c_str());
}

bool free_dll(dll_handle dll)
{
    return FreeLibrary(dll);
}

template<typename fn_t = void(*)(void)>
fn_t get_fn(dll_handle dll, std::string fn_name)
{
    if (dll) return (fn_t)GetProcAddress(dll, fn_name.c_str());
    return nullptr;
}

int dll_counter = 0;
dll_handle compile_dll(std::vector<std::string> src_files)
{
    std::ostringstream dll{};
    std::ostringstream cmd{};
    dll << base_filename << ++dll_counter << ".dll";
    cmd << "gcc -shared -o " << dll.str();
    for (std::string& file_name : src_files) cmd << " " << file_name;
    system(cmd.str().c_str());
    return load_dll(dll.str());
}

// extern "C" wrapper to prevent c++ name mangling
std::string extern_c_header = "\n#ifdef __cplusplus\nextern \"C\" {\n#endif\n";
std::string extern_c_footer = "\n#ifdef __cplusplus\n}\n#endif\n";

int src_counter = 0;
std::string create_src(std::vector<std::string> includes, std::vector<std::string> lines)
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

void clean_up()
{
    std::ostringstream oss{};
    oss << del_cmd << " " << base_filename << "*";
    oss << " 2>nul"; // suppress error messages
    auto r = system(oss.str().c_str());
    std::cout << "system returned " << r << std::endl;
}

#endif
