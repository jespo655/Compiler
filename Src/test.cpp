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
#include "abstx/all_abstx.h"
#include "types/cb_types.h"
#include "utilities/assert.h"
// #include "parser/parser.h"
#include "lexer/lexer.h"

#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <memory>
using namespace std;



// struct Generic_type {
//     CB_Type type; // each generic type is different
//     std::string name;
//     Generic_type() {}
//     Generic_type(const std::string& name) : type{name}, name{name} {}
// };

// struct Generic_statement {
//     Statement s; // contains some Generic_types instead of types
// };

void any_test()
{
    // cout << "~ any test ~" << endl;
    // CB_Any any;      // cout << any.toS() << endl;
    // any = CB_Int(2); // cout << any.toS() << endl;
    // any.~CB_Any();   // cout << any.toS() << endl;
    // CB_Any any2 = CB_Float(2); // cout << any2.toS() << endl;
    // any = any2;      // cout << any.toS() << endl;
    // any = CB_Int(2); // cout << any.toS() << endl;
    // CB_Int i = any.value<CB_Int>(); // cout << i.toS() << endl;


    // cout << "1"; const CB_Owning_pointer<CB_Int> ptr;
    // cout << "2"; const CB_Any any_ptr = ptr; // error move
    // cout << "3"; any_ptr = std::move(ptr); // ok

    // cout << "4"; CB_Any any_ptr2 = any_ptr; // error move
    // cout << "5"; any_ptr2 = std::move(any_ptr); // ok
    // cout << "~ any test complete ~" << endl;

}


void flag_test()
{

    CB_Flag f0 = 0;
    CB_Flag f1 = 1;
    CB_Flag f2 = 2;
    CB_Flag f3 = 3;
    CB_Flag f4 = 4;
    CB_Flag f5 = 5;
    CB_Flag f10 = 10;
    CB_Flag f16 = 16;
    CB_Flag f64 = 64;

    CB_u8 flags1 = f0+f2;
    CB_u16 flags2 = (CB_Uint)f16;

    cout << flags1.toS() << endl;
    cout << flags2.toS() << endl;
    // cout << (1<<(0U-1)) << endl;
    // cout << CB_Uint(1ULL<<63) << endl;
    // cout << CB_Uint(1ULL<<64) << endl;
    // cout << CB_Uint(1ULL<<65) << endl;
}

















struct Thing {
    static Thing member;
};
Thing Thing::member = Thing();

template<typename T, typename M=Thing*, M=&T::member>
void foo(T t) { cout << "obj" << endl; }

template<typename T, typename M=Thing*, M=&T::member>
void foo(T* t) { cout << "ptr" << endl; }

// template<typename T, typename M=int*, M=&T::member>
// void foo(T* t) { cout << "ptr" << endl; }

// template<typename T>
// void foo(T t) { cout << "obj" << endl; }

// template<typename T>
// void foo(T* t) { cout << "ptr" << endl; }


void template_test()
{
    Thing obj = Thing();
    Thing* ptr = new Thing();

    foo(obj);
    foo(ptr);

    delete ptr;
}






struct AB {
    char a, b;
};



// void plus_i8(CB_i8 lhs, CB_i8 rhs, CB_i8& sum) { // references doesn't work for some reason
//     cout << "hej?" << endl;
//     cout << "lhs = " << lhs.toS() << endl;
//     cout << "rhs = " << rhs.toS() << endl;
//     cout << "sum = " << lhs.v+rhs.v << endl;
//     sum.v = lhs.v+rhs.v;
//     cout << "hej då " << endl;
// }

void plus_ints(CB_i8 lhs, CB_i16 rhs, CB_i32* sum) {
    sum->v = lhs.v+rhs.v;
}



struct Verbose
{
    int i = CB_Type::get_unique_type_id();
    Verbose() { cout << "V " << i << " constructed" << endl; }
    ~Verbose() { cout << "V " << i << " deleted" << endl; }
    Verbose(const Verbose& v) { cout << "V " << i << " copy constructor from " << v.i << endl; }
    Verbose& operator=(const Verbose& v) { cout << "V " << i << " copy assignment from " << v.i << endl; }
    Verbose(Verbose&& v) { cout << "V " << i << " move constructor from " << v.i << endl; }
    Verbose& operator=(Verbose&& v) { cout << "V " << i << " move assignment from " << v.i << endl; }
};


CB_Owning_pointer<Verbose> create_verbose()
{
    cout << "creating" << endl;
    CB_Owning_pointer<Verbose> vp = alloc(Verbose());
    cout << "returning" << endl;
    return vp;
}

void grab_verbose(CB_Sharing_pointer<Verbose> vp) {
    cout << "grabbed " << vp->i << endl;
    cout << "returning" << endl;
}


void owning_test()
{
    // Verbose v = Verbose();
    CB_Owning_pointer<Verbose> vp = create_verbose();
    cout << "vp: " << vp.toS() << endl;
    cout << "v.i: " << vp->i << endl;
    grab_verbose(std::move(vp));
    cout << "vp: " << vp.toS() << endl;
}


void range_test()
{
    CB_Range r;
    r.r_start = 2.1;
    r.r_end = 10.4;
    cout << "range: " << r.toS() << endl;
    for (auto f : r) {
        cout << f.toS() << endl;
    }
}

void print_types()
{
    cout << CB_Type::type.toS() << ": CB_Type, size = " << sizeof(CB_Type) << ", default value = " << CB_Type::type.default_value().toS() << endl;
    cout << CB_String::type.toS() << ": CB_String, size = " << sizeof(CB_String) << ", default value = " << CB_String::type.default_value().toS() << endl;
    cout << CB_Bool::type.toS() << ": CB_Bool, size = " << sizeof(CB_Bool) << ", default value = " << CB_Bool::type.default_value().toS() << endl;

    cout << CB_i8::type.toS() << ": CB_i8, size = " << sizeof(CB_i8) << ", default value = " << CB_i8::type.default_value().toS() << endl;
    cout << CB_i16::type.toS() << ": CB_i16, size = " << sizeof(CB_i16) << ", default value = " << CB_i16::type.default_value().toS() << endl;
    cout << CB_i32::type.toS() << ": CB_i32, size = " << sizeof(CB_i32) << ", default value = " << CB_i32::type.default_value().toS() << endl;
    cout << CB_i64::type.toS() << ": CB_i64, size = " << sizeof(CB_i64) << ", default value = " << CB_i64::type.default_value().toS() << endl;

    cout << CB_u8::type.toS() << ": CB_u8, size = " << sizeof(CB_u8) << ", default value = " << CB_u8::type.default_value().toS() << endl;
    cout << CB_u16::type.toS() << ": CB_u16, size = " << sizeof(CB_u16) << ", default value = " << CB_u16::type.default_value().toS() << endl;
    cout << CB_u32::type.toS() << ": CB_u32, size = " << sizeof(CB_u32) << ", default value = " << CB_u32::type.default_value().toS() << endl;
    cout << CB_u64::type.toS() << ": CB_u64, size = " << sizeof(CB_u64) << ", default value = " << CB_u64::type.default_value().toS() << endl;

    cout << CB_f32::type.toS() << ": CB_f32, size = " << sizeof(CB_f32) << ", default value = " << CB_f32::type.default_value().toS() << endl;
    cout << CB_f64::type.toS() << ": CB_f64, size = " << sizeof(CB_f64) << ", default value = " << CB_f64::type.default_value().toS() << endl;

    cout << CB_Int::type.toS() << ": CB_Int, size = " << sizeof(CB_Int) << ", default value = " << CB_Int::type.default_value().toS() << endl;
    cout << CB_Uint::type.toS() << ": CB_Uint, size = " << sizeof(CB_Uint) << ", default value = " << CB_Uint::type.default_value().toS() << endl;
    cout << CB_Float::type.toS() << ": CB_Float, size = " << sizeof(CB_Float) << ", default value = " << CB_Float::type.default_value().toS() << endl;

    cout << CB_Any::type.toS() << ": CB_Any, size = " << sizeof(CB_Any) << endl; // ", default value = " << CB_Any::type.default_value().toS() << endl;

    cout << CB_Range::type.toS() << ": CB_Range, size = " << sizeof(CB_Range) << ", default value = " << CB_Range::type.default_value().toS() << endl;
    cout << CB_Function_type::type.toS() << ": CB_Function_type, size = " << sizeof(CB_Function_type) << ", default value = " << CB_Function_type::type.default_value().toS() << endl;
    cout << CB_Function::type.toS() << ": CB_Function, size = " << sizeof(CB_Function) << ", default value = " << CB_Function::type.default_value().toS() << endl;
    CB_Function fn = CB_Function::type.default_value().value<CB_Function>();
    cout << "CB_Function default value: " << fn.toS() << endl;
    fn = nullptr;
    cout << "CB_Function after nullptr assignment: " << fn.toS() << endl;
    cout << CB_Function::type.toS() << ": CB_Function, size = " << sizeof(CB_Function) << ", default value = " << CB_Function::type.default_value().toS() << endl;
    // cout << CB_Generic_function::type.toS() << ": CB_Generic_function, size = " << sizeof(CB_Generic_function) << endl; // ", default value = " << CB_Generic_function::type.default_value().toS() << endl;
    // cout << CB_Operator::type.toS() << ": CB_Operator, size = " << sizeof(CB_Operator) << ", default value = " << endl; //CB_Operator::type.default_value().toS() << endl;

    cout << CB_Dynamic_seq<CB_Int>::type.toS() << ": CB_Dynamic_seq<CB_Int>, size = " << sizeof(CB_Dynamic_seq<CB_Int>) << ", default value = " << CB_Dynamic_seq<CB_Int>::type.default_value().toS() << endl;
    cout << CB_Static_seq<CB_Int, 5>::type.toS() << ": CB_Static_seq<CB_Int, 5>, size = " << sizeof(CB_Static_seq<CB_Int, 5>) << ", default value = " << CB_Static_seq<CB_Int, 5>::type.default_value().toS() << endl;

    cout << CB_Owning_pointer<CB_Int>::type.toS() << ": CB_Owning_pointer<CB_Int>, size = " << sizeof(CB_Owning_pointer<CB_Int>) << ", default value = " << CB_Owning_pointer<CB_Int>::type.default_value().toS() << endl;
    cout << CB_Sharing_pointer<CB_Int>::type.toS() << ": CB_Sharing_pointer<CB_Int>, size = " << sizeof(CB_Sharing_pointer<CB_Int>) << ", default value = " << CB_Sharing_pointer<CB_Int>::type.default_value().toS() << endl;

    // cout << CB_Struct<CB_Int>::type.toS() << ": CB_Struct<CB_Int>, size = " << sizeof(CB_Struct<CB_Int>) << endl;

    cout << endl;
}




void cb_fn_test()
{
    CB_Function fn;
    fn.v = (void (*)())plus_ints;

    CB_Function_type fn_type;
    fn_type.set_in_args<CB_i8, CB_i16>();
    fn_type.set_out_args<CB_i32>();

    fn.fn_type = &fn_type;

    CB_i8 a8 = 2;
    CB_i16 b8 = 5;
    CB_i32 c8;
    fn(a8,b8,&c8);
    cout << dec << a8.toS() << " + " << b8.toS() << " = " << c8.toS() << endl;
}



void cb_types_test()
{
    // CB_i8 i;
    // CB_String s;
    // CB_Struct<CB_i8, CB_String, CB_i16> s{}; // no init on str is likely to crash
    // CB_Struct<> s2{}; // no init on str is likely to crash

    // s.get<CB_i8>(0).v = 1;
    // s.get<CB_i16>(2).v = 4;
    // s.get<CB_String>(1) = "9"; // string("9");

    // cout << "i8: " << s.get<CB_i8>(0).toS() << endl;
    // cout << "i16: " << s.get<CB_i16>(2).toS() << endl;
    // cout << "string: " << s.get<CB_String>(1).toS() << endl;

    // cout << "s1: " << s.toS() << endl;
    // cout << "s2: " << s2.toS() << endl;

    cout << endl;

    cout << "size i8: " << sizeof(CB_i8) << endl;
    // cout << "size value: " << sizeof(CB_Value) << endl;
    cout << "size type: " << sizeof(CB_Type) << endl;
    cout << "size bool: " << sizeof(CB_Bool) << endl;
    cout << "size i16: " << sizeof(CB_i16) << endl;
    cout << "size string: " << sizeof(CB_String) << endl;
    // cout << "size pointer: " << sizeof(CB_Owning_pointer<Struct_metadata>) << endl;
    // cout << "size struct real: " << sizeof(s) << endl;
    // cout << "size struct approx: " << sizeof_struct<CB_i8, CB_String, CB_i16>() <<
    //     " + " << sizeof(CB_Owning_pointer<Struct_metadata>) << " = " <<
    //     sizeof_struct<CB_i8, CB_String, CB_i16>() + sizeof(CB_Owning_pointer<Struct_metadata>) << endl;

    cout << "cpp  tests: " << sizeof(AB) << endl;
    // cout << "cube tests: " << sizeof(CB_Struct<CB_i8, CB_i8>) << endl;
    // cout << "cube tests: " << sizeof(CB_Struct<>) << endl;
    cout << "cube tests: " << sizeof(CB_Range) << endl;


    CB_f32 f = 2;
    CB_Bool b = true;


    // CB_Function fn;
    // CB_Function<Args<CB_i8, CB_i8>, Args<CB_i8>> fn;
    // cout << "fn size: " << sizeof(CB_Function) << endl;
    // cout << "fn type: " << fn.type.toS() << endl;

    // fn.v = (void (*)())plus_i8;
    // CB_i8 a8 = 2;
    // CB_i8 b8 = 5;
    // CB_i8 c8;
    // fn(a8,b8,c8);


    // fn.v = (void (*)())plus_ints;
    // fn.set_in_args<CB_i8, CB_i16>();
    // fn.set_out_args<CB_i32>();

    CB_i8 a8 = 2;
    CB_i16 b8 = 5;
    CB_i32 c8;
    // fn(a8,b8,&c8);
    cout << dec << a8.toS() << " + " << b8.toS() << " = " << c8.toS() << endl;


    // typedef Args<Generic_arg<0>> T1;
    // typedef Generic_arg<1> T2;
    // CB_Generic_function<T1, T2> gen_fn2;
    // cout << "genfn2 type: " << gen_fn2.type.toS() << endl;

    // CB_Generic_function<Args<Generic_arg<0>>, Args<Generic_arg<0>>> gen_fn;
    // cout << "gen fn size: " << sizeof(CB_Range) << endl;
    // cout << "gen fn type: " << fn.type.toS() << endl;

    // auto spec_fn = gen_fn.get_specialization<Args<CB_i8>, Args<CB_i8>>();
    // CB_Function<Args<CB_i8>, Args<CB_i8>> spec_fn = *gen_fn.get_specialization<Args<CB_i8>, Args<CB_i8>>();

    // auto spec_fn = gen_fn.get_specialization<Args<CB_i8>, Args<CB_i8>>();

    // get_fn<Arg<CB_i8>, typename T>()
    // CB_Function<Args<CB_i8>, Args<CB_i8>>::Return_type a;


    // cpp_struct<int, float, int> s{{"a", "b", "c"}};
    // s.v = CB_Struct<int, float, int>(make_tuple(1, 2.5, 2));
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






void jocke_test()
{
    cout << "hej" << endl;

    int a = 2;
    int b = 3;
    int c = a + b;

    cout << c << endl;


    CB_Dynamic_seq<CB_Int> seq;
    for (int i = 0; i < 10; ++i)
    {
        seq.add(i);
    }
    cout << "listan innehåller: ";
    for (CB_Int i : seq) {
        cout << i << " ";
    }
    cout << endl;

    cout << (char)0b00111111 << endl;
    cout << (int)0b00111111 << endl;
}



template<typename T, typename Type=CB_Type*, Type=&T::type>
void generic_type_test(const T& t) {
    // std::cout << "any move op = " << T::type.toS() << std::endl;
    ASSERT(T::type != CB_Any::type);
    cout << "t: " << t.toS();
    cout << "type: " << t.type.toS() << endl;
}

/*
void struct_test() {

    cout << endl << " ------------------ TEST BEGIN ------------------ " << endl;

    Struct_type Vector3{"Vector3"};
    Vector3.metadata->members.add(Struct_member("x", CB_Int::type, CB_Int()));
    Vector3.metadata->members.add(Struct_member("y", CB_Int::type, CB_Int()));
    Vector3.metadata->members.add(Struct_member("z", CB_Int::type, CB_Int()));
    cout << "Type Vector3: " << Vector3.toS() << endl;

    Struct_instance v3{Vector3};
    cout << endl << "Instance of Vector3: " << v3 << endl;
    v3.member("x") = CB_Int(1);
    v3.member("y") = CB_Int(4);
    v3.member("z") = CB_Int(9);
    cout << "Instance of Vector3 given values: " << v3 << endl;

    CB_Any any3 = v3;
    cout << "CB_Any holding a struct: " << any3.toS() << endl;

    Struct_instance copy3 = v3;
    cout << "after copy assignment: " << endl;
    cout << "Old instance: " << v3.toS() << endl;
    cout << "New instance: " << copy3.toS() << endl;

    copy3 = std::move(v3);
    cout << "after move assignment: " << endl;
    // cout << "Old instance: " << v3.toS() << endl; // crashes
    cout << "New instance: " << copy3.toS() << endl;


    Struct_type Vector4{"Vector4"};
    Vector4.metadata->members.add(Struct_member("v3", Vector3, Vector3(), true));
    // Vector4.metadata->members.add(Struct_member("u", Vector3, Vector3(), true)); // using=true
    Vector4.metadata->members.add(Struct_member("w", CB_Int::type, CB_Int()));
    cout << "Type Vector4: " << Vector4.toS() << endl;

    Struct_instance v4{Vector4};
    cout << endl << "Instance of Vector4: " << v4 << endl;
    v4.member("v3").value<Struct_instance>().member("x") = CB_Int(1);
    v4.member("v3").value<Struct_instance>().member("y") = CB_Int(1);
    v4.member("v3").value<Struct_instance>().member("z") = CB_Int(1);
    v4.member("x") = CB_Int(2);
    v4.member("y") = CB_Int(2);
    v4.member("z") = CB_Int(2);
    v4.member("w") = CB_Int(3);
    cout << "Instance of Vector4 given values: " << v4 << endl;

    CB_Any any4 = v4;
    cout << "CB_Any holding a struct: " << any4.toS() << endl;

    Struct_instance copy4 = v4;
    cout << "after copy assignment: " << endl;
    cout << "Old instance: " << v4.toS() << endl;
    cout << "New instance: " << copy4.toS() << endl;

    copy4 = std::move(v4);
    cout << "after move assignment: " << endl;
    // cout << "Old instance: " << v4.toS() << endl; // chrashes
    cout << "New instance: " << copy4.toS() << endl;

    cout << " ------------------ TEST END ------------------ " << endl << endl;

}
*/


void compile_test()
{
    // std::shared_ptr<Global_scope> gs = parse_file("../Demos/helloworld.cb");
    std::vector<Token> hw_tokens = get_tokens_from_file("../Demos/helloworld.cb");
    std::cout << "../Demos/helloworld.cb tokens: " << endl;
    for (const Token& token : hw_tokens) {
        std::cout << token.token << " ";
    }
    std::cout << endl;


    std::vector<Token> code_tokens = get_tokens_from_file("lexer/code.cb");
    std::cout << "lexer/code.cb tokens: " << endl;
    for (const Token& token : code_tokens) {
        std::cout << token.token << " ";
    }
    std::cout << endl;

}

void code_gen_test()
{
    // first: get tokens from file
    std::vector<Token> hw_tokens = get_tokens_from_file("../Demos/helloworld.cb");
    std::cout << "../Demos/helloworld.cb tokens: " << endl;
    for (const Token& token : hw_tokens) {
        std::cout << token.token << " ";
    }
    std::cout << endl;

    // compile into abstx tree
    // TODO

    // get entry point
    // Function entry_point;
    // std::set<std::string> c_sources;
    // generate_code(std::cout, &fn, c_sources);
}

void function_test()
{
    std::cout << "test start" << std::endl;
    CB_Function_type fn_t;
    std::cout << "adding arg types" << std::endl;
    fn_t.in_types.add(CB_Sharing_pointer<CB_Type>(&CB_Int::type));
    fn_t.in_types.add(CB_Sharing_pointer<CB_Type>(&CB_Int::type));
    fn_t.out_types.add(CB_Sharing_pointer<CB_Type>(&CB_Int::type));
    std::cout << "fn type: " << fn_t.toS() << std::endl;

    ASSERT(fn_t != CB_Int::type, "fn type is not the same as int!");
    ASSERT(CB_Int::type != fn_t, "fn type is not the same as int!");
    ASSERT(fn_t == fn_t);

    CB_Function_type fn_t2;
    fn_t2.in_types.add(CB_Sharing_pointer<CB_Type>(&CB_i8::type));
    fn_t2.in_types.add(CB_Sharing_pointer<CB_Type>(&CB_Int::type));
    fn_t2.out_types.add(CB_Sharing_pointer<CB_Type>(&CB_Int::type));

    ASSERT(fn_t != fn_t2);

    CB_Function_type fn_t3;
    fn_t3.in_types.add(CB_Sharing_pointer<CB_Type>(&CB_Int::type));
    fn_t3.in_types.add(CB_Sharing_pointer<CB_Type>(&CB_Int::type));
    fn_t3.out_types.add(CB_Sharing_pointer<CB_Type>(&CB_Int::type));

    ASSERT(fn_t == fn_t3);
}


int main()
{
    // print_types();
    // Debug_os os{std::cout};
    // ptr_reference_test();
    // unique_id_test();
    // size_test();
    // indent_test();
    // float_test();
    // wchar_test();
    // str_test();
    // seq_test();
    // cb_types_test();
    // owning_test();
    // template_test();
    // cb_fn_test();
    // range_test();
    // any_test();
    // flag_test();
    // jocke_test();
    // struct_test();
    // compile_test();

    function_test();
    // code_gen_test();
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
