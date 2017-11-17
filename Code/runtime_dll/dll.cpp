

#include <windows.h>
#include <string>
#include <iostream> // debug
#include <sstream>
#include <vector>
#include <fstream>

// returns null if dll can't be found
HINSTANCE load_dll(std::string filename)
{
    return LoadLibrary(TEXT(filename.c_str()));
}

template<typename fn_t = void(*)(void)>
fn_t get_fn(HINSTANCE dll, std::string fn_name)
{
    if (dll) return (fn_t)GetProcAddress(dll, fn_name.c_str());
    return nullptr;
}


void compile_dll(std::vector<std::string> src_files)
{
    std::ostringstream oss{};
    oss << "g++ -shared -o tmp.dll";
    for (std::string& file_name : src_files) oss << " " << file_name;
    system(oss.str().c_str());
}

typedef void (*dll_fn)();



bool test_dll()
{
    compile_dll({"tmp.cpp"});
    auto dll = load_dll("tmp.dll");
    if (dll == nullptr) return false;
    auto fn = get_fn<void(*)(char*)>(dll, "test_str");
    if (fn == nullptr) return false;
    char s[] = "hej";
    fn(s);
    return true;
}

std::string fn1_text = "void test() { printf(\"CUSTOM DLL FUNCTION\\n\"); }";
std::string fn2_text = "void test_str(char const* str) { printf(\"CUSTOM DLL FUNCTION: %s\\n\", str); }";
std::string extern_c_header = "#ifdef __cplusplus\nextern \"C\" {\n#endif\n";
std::string extern_c_footer = "}\n";

void create_src(std::vector<std::string> includes, std::vector<std::string> lines)
{
    std::ofstream ofs{"tmp.cpp"};
    for (std::string& include : includes) ofs << "#include \"" << include << "\"\n" << std::endl;;
    ofs << extern_c_header << std::endl;
    for (std::string& line : lines) ofs << line << std::endl;;
    ofs << extern_c_footer << std::endl;
    ofs.close();
}




typedef int (__cdecl *MYPROC)(LPWSTR);

int main( void )
{
    create_src({"stdio.h"}, {fn1_text, fn2_text});
    if (test_dll()) printf("success!\n");
    else printf("fail");
    return 0;


    HINSTANCE hinstLib;
    MYPROC ProcAdd;
    BOOL fFreeResult, fRunTimeLinkSuccess = FALSE;
    wchar_t msg_sent_str[] = L"Message sent to the DLL function\n";

    // Get a handle to the DLL module.

    hinstLib = load_dll("lib.dll");
    if (hinstLib == nullptr) printf("dll instance is null\n");

    // If the handle is valid, try to get the function address.

    if (hinstLib != NULL)
    {
        // ProcAdd = get_fn<MYPROC>(hinstLib, "myPuts"); GetProcAddress(hinstLib, "myPuts");
        ProcAdd = get_fn<MYPROC>(hinstLib, "test"); // GetProcAddress(hinstLib, "myPuts");
        if (ProcAdd == nullptr) printf("function is null\n");

        // If the function address is valid, call the function.

        if (NULL != ProcAdd)
        {
            fRunTimeLinkSuccess = TRUE;
            (ProcAdd) (msg_sent_str);
            printf("everything is fine\n");
        }
        // Free the DLL module.

        fFreeResult = FreeLibrary(hinstLib);
    }

    // If unable to call the DLL function, use an alternative.
    if (! fRunTimeLinkSuccess)
        printf("Message printed from executable\n");

    return 0;

}

