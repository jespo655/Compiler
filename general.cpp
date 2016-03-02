#include <vector>
#include <string>
#include <map>
using namespace std;









// actual types:

// struct String
// {
//     char* value;
// };

// struct Pointer
// {
//     Type_information target_type;
//     // value?
// };

// struct Function
// {
//     vector<Type_information> parameters;
//     vector<Type_information> return_values;
//     // body?
// };








// // a list of typenames. Can be expanded with used defined types
// map<string,Type_information> types[] =
// {
//     { "u8", { "u8", 1 } },
//     { "u16", { "u16", 2 } },
//     { "u32", { "u32", 4 } },
//     { "u64", { "u64", 8 } },
//     { "i8", { "i8", 1 } },
//     { "i16", { "i16", 2 } },
//     { "i32", { "i32", 4 } },
//     { "i64", { "i64", 8 } },
//     { "float", { "float", 4 } },
//     { "double", { "double", 8} },
//     { "string", { "string", sizeof(String) } },
//     { "_pointer", { "_pointer", sizeof(Pointer) } },
//     // { "_function", { "_function", sizeof(Function) } } // A function type is always user defined. The type name will be a mangled version of the function signature.
// };

// void add_type(const Type_information& t)
// {
//     types[t.type_name] = t;
// }




/*
vector<string> keywords
{
    "cast",
    "return",
    "if",
    "else",
    "for", // iteration över lista eller range
    "while", // så länge ett condition håller
    // allt som börjar med _
};
*/

/*
struct Context
{
    Token* begin;
    Token* end;
};
*/
