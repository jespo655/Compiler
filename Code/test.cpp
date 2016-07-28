// #include "abstx/abstx.h"
// #include "abstx/function.h"
// #include "abstx/identifier.h"
// #include "abstx/if.h"
// #include "abstx/numbers.h"
// #include "abstx/literal.h"
// #include "abstx/scope.h"
// #include "abstx/statement.h"
// #include "abstx/type.h"
// #include "compile_time/workspace.h"
// #include "utilities/unique_id.h"
// #include "utilities/assert.h"
#include "abstx/cb_types.h"
#include "utilities/assert.h"


#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <memory>
using namespace std;




struct AB {
    char a, b;
};




void cb_types_test()
{
    // CB_i8 i;
    // CB_String s;
    CB_struct<CB_i8, CB_String, CB_i16> s{}; // no init on str is likely to crash
    CB_struct<> s2{}; // no init on str is likely to crash

    // s.get<CB_i8>(0).v = 1;
    // s.get<CB_i16>(2).v = 4;
    s.get<CB_String>(1) = "9"; // string("9");

    cout << "i8: " << s.get<CB_i8>(0).toS() << endl;
    cout << "i16: " << s.get<CB_i16>(2).toS() << endl;
    cout << "string: " << s.get<CB_String>(1).toS() << endl;

    cout << "s1: " << s.toS() << endl;
    cout << "s2: " << s2.toS() << endl;

    cout << endl;

    cout << "size i8: " << sizeof(CB_i8) << endl;
    // cout << "size value: " << sizeof(CB_Value) << endl;
    cout << "size type: " << sizeof(CB_Type) << endl;
    cout << "size bool: " << sizeof(CB_bool) << endl;
    cout << "size i16: " << sizeof(CB_i16) << endl;
    cout << "size string: " << sizeof(CB_String) << endl;
    cout << "size pointer: " << sizeof(CB_owning_pointer<Metadata>) << endl;
    cout << "size struct real: " << sizeof(s) << endl;
    cout << "size struct approx: " << sizeof_struct<CB_i8, CB_String, CB_i16>() <<
        " + " << sizeof(CB_owning_pointer<Metadata>) << " = " <<
        sizeof_struct<CB_i8, CB_String, CB_i16>() + sizeof(CB_owning_pointer<Metadata>) << endl;

    cout << "cpp  tests: " << sizeof(AB) << endl;
    cout << "cube tests: " << sizeof(CB_struct<CB_i8, CB_i8>) << endl;
    cout << "cube tests: " << sizeof(CB_struct<>) << endl;
    cout << "cube tests: " << sizeof(CB_range) << endl;


    CB_f32 f = 2;
    CB_bool b = true;



    // cpp_struct<int, float, int> s{{"a", "b", "c"}};
    // s.v = CB_struct<int, float, int>(make_tuple(1, 2.5, 2));
    // // s.identifiers = {"a", "b", "c"};

    // s.get_member("a");
    // // cout << "a: " << s.get_member("a") << endl;
    // // cout << "b: " << s.get_member("b") << endl;
    // // cout << "c: " << s.get_member("c") << endl;

    // tuple<int, int> t = make_tuple(1, 2);
    // cout << get<1>(t) << endl;
    // cout << get<1>(s.v.v) << endl;
    // cout << s.get<1>() << endl;
}













struct Dummy {
    int i;
    int get_i() {
        return i;
    }
};

ostream& operator<<(ostream& os, const Dummy& d)
{
    return os << d.i;
}

template<typename Seq_t>
void seq_generic_test(Seq_t& seq)
{
    cout << "test begin" << endl;
    cout << "sizeof(seq) = " << sizeof(seq) << endl;
    int min = 5;
    int max = 100;
    for (int i = min; i < max; ++i) {
        // ostringstream oss;
        // oss << i;
        // seq.set(i, oss.str());
        // seq.set(i, i);
        Dummy d;
        d.i = i;
        seq.set(i, d);
        if (i%10 == 0) cout << "size: " << seq.get_size() << ", capacity: " << seq.get_capacity() << endl;
    }
    for (int i = min-1; i < max+1; ++i)
        cout << "seq[" << i << "] = " << seq[i] << endl;
    cout << "sizeof(seq) = " << sizeof(seq) << endl;

    cout << "test complete" << endl;
}


void seq_test()
{
    // CB_Static_seq<Dummy, 50> s_seq;
    // CB_Dynamic_seq<Dummy> d_seq;

    // // CB_Static_seq<int, 5> s_seq;
    // // CB_Dynamic_seq<int> d_seq;

    // seq_generic_test(s_seq);
    // seq_generic_test(d_seq);

    // cout << "size = " << sizeof(CB_Static_seq<int, 0>);
}











void str_test()
{
    std::string s = "asd";
    int len = s.size();

    // char v[len+1];
    char* v = (char*)malloc(len+1);

    for (int i = 0; i < s.size(); ++i)
        v[i] = s[i];
    v[len] = '\0';

    cout << "len=" << len << endl;
    cout << "v=" << v << endl;

    delete v;

}



void ptr_reference_test()
{
    // Debug_os os{std::cout};


    // auto sp1 = shared_ptr<Scope>(new Scope());
    // auto sp2 = shared_ptr<Scope>(new Scope());
    // sp2->dynamic = true;

    // Identifier id{};
    // id.parent_scope = sp1;
    // id.parent_scope.lock()->debug_print(os);

    // id.owner.lock()->debug_print(os);

    // id.owner = sp2;
    // id.parent_scope.lock()->debug_print(os);
    // id.owner.lock()->debug_print(os);

    // cout << id.parent_scope.lock() << endl;
    // cout << id.owner.lock() << endl;
}


void unique_id_test()
{
    // cout << get_unique_id() << endl;
    // cout << get_unique_id() << endl;
    // cout << get_unique_id() << endl;
    // cout << get_unique_id() << endl;
    // cout << get_unique_id() << endl;
    // cout << "---------------" << endl;
    // test_unique_id();
    // cout << "---------------" << endl;
    // cout << get_unique_id() << endl;
    // cout << get_unique_id() << endl;
    // cout << "---------------" << endl;
    // test_unique_id();
}

void indent_test()
{
    // Debug_os os{std::cout};
    // Scope s{};
    // s.debug_print(os);
    // os.indent();
    // s.debug_print(os);
    // os.indent();
    // s.debug_print(os);
    // os.indent();
    // s.debug_print(os);
    // os.unindent();
    // s.debug_print(os);
    // os.unindent();
    // s.debug_print(os);
    // cout << "done!" << endl;
}

void size_test()
{
    // cout << "bool: " << sizeof(bool) << endl;
    // cout << "int8_t: " << sizeof(int8_t) << endl;
    // cout << "int16_t: " << sizeof(int16_t) << endl;
    // cout << "int32_t: " << sizeof(int32_t) << endl;
    // cout << "int64_t: " << sizeof(int64_t) << endl;

    // cout << "uint8_t: " << sizeof(uint8_t) << endl;
    // cout << "uint16_t: " << sizeof(uint16_t) << endl;
    // cout << "uint32_t: " << sizeof(uint32_t) << endl;
    // cout << "uint64_t: " << sizeof(uint64_t) << endl;

    // cout << "float: " << sizeof(float) << endl;
    // cout << "double: " << sizeof(double) << endl;

    cout << "unique size = " << sizeof(std::unique_ptr<int>) << endl;
    cout << "shared size = " << sizeof(std::shared_ptr<int>) << endl;
    cout << "bool size = " << sizeof(bool);

}


void float_test()
{
    // todo: kolla om float skrivs ut med eller utan decimaler om de är heltal
    cout << 0 << endl;
    cout << 0.0 << endl;

    int i = 0;
    float f = 1.1;
    double d = 0;
    cout << i << f << d << endl;
    f -= 0.1;
    cout << f << endl;
    f = 2.2;
    cout << f/1.1 << endl;
}




void wchar_test() {
    string s = "a¤";
    char c = s[0];
    wchar_t wc = s[0];

    cout << " s=" << s << ", len=" << s.size() << endl;
    cout << " c=" << c << ", size=" << sizeof(char) << endl;
    cout << "wc=" << wc << ", size=" << sizeof(wchar_t) << endl;
    wcout << "wc=" << wc << ", size=" << sizeof(wchar_t) << endl;

    string s2;
    cin >> s2;

    cout << "s2=" << s2 << ", len=" << s2.size() << endl;
    cout << "equal to s = " << (s==s2) << endl;
    c = s[0];
    wc = s[0];

    cout << " c=" << c << ", size=" << sizeof(char) << endl;
    cout << "wc=" << wc << ", size=" << sizeof(wchar_t) << endl;
    wcout << "wc=" << wc << ", size=" << sizeof(wchar_t) << endl;
}



int main()
{
    // Debug_os os{std::cout};
    // ptr_reference_test();
    // unique_id_test();
    // size_test();
    // indent_test();
    // float_test();
    // wchar_test();
    // str_test();
    // seq_test();
    cb_types_test();
}






void size_assertions()
{
    // ASSERT(sizeof(int8_t) == 1);
    // ASSERT(sizeof(int16_t) == 2);
    // ASSERT(sizeof(int32_t) == 4);
    // ASSERT(sizeof(int64_t) == 8);

    // ASSERT(sizeof(uint8_t) == 1);
    // ASSERT(sizeof(uint16_t) == 2);
    // ASSERT(sizeof(uint32_t) == 4);
    // ASSERT(sizeof(uint64_t) == 8);

    // ASSERT(sizeof(float) == 4);
    // ASSERT(sizeof(double) == 8);
}