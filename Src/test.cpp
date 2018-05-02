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
#include "types/all_cb_types.h"
#include "utilities/assert.h"
// #include "parser/parser.h"
#include "lexer/lexer.h"

#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <memory>
using namespace std;



void flag_test()
{

    flag f0 = 0;
    flag f1 = 1;
    flag f2 = 2;
    flag f3 = 3;
    flag f4 = 4;
    flag f5 = 5;
    flag f10 = 10;
    flag f16 = 16;
    flag f64 = 64;

    uint8_t flags1 = f0+f2;
    uint16_t flags2 = (uint64_t)f16;

    cout << flags1 << endl;
    cout << flags2 << endl;
    // cout << (1<<(0U-1)) << endl;
    // cout << uint64_t(1ULL<<63) << endl;
    // cout << uint64_t(1ULL<<64) << endl;
    // cout << uint64_t(1ULL<<65) << endl;
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


Owned<Verbose> create_verbose()
{
    cout << "creating" << endl;
    Owned<Verbose> vp = alloc(Verbose());
    cout << "returning" << endl;
    return vp;
}

void grab_verbose(Shared<Verbose> vp) {
    cout << "grabbed " << vp->i << endl;
    cout << "returning" << endl;
}


void owning_test()
{
    // Verbose v = Verbose();
    Owned<Verbose> vp = create_verbose();
    cout << "vp: " << vp.toS() << endl;
    cout << "v.i: " << vp->i << endl;
    grab_verbose(std::move(vp));
    cout << "vp: " << vp.toS() << endl;
}


void range_test()
{
    range r;
    r.r_start = 2.1;
    r.r_end = 10.4;
    cout << "range: " << r.toS() << endl;
    for (auto f : r) {
        cout << f << endl;
    }
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
void seq_generic_test(Seq_t& Seq)
{
    cout << "test begin" << endl;
    cout << "sizeof(Seq) = " << sizeof(Seq) << endl;
    int min = 5;
    int max = 100;
    for (int i = min; i < max; ++i) {
        // ostringstream oss;
        // oss << i;
        // Seq.set(i, oss.str());
        // Seq.set(i, i);
        Dummy d;
        d.i = i;
        Seq.set(i, d);
        if (i%10 == 0) cout << "size: " << Seq.get_size() << ", capacity: " << Seq.get_capacity() << endl;
    }
    for (int i = min-1; i < max+1; ++i)
        cout << "Seq[" << i << "] = " << Seq[i] << endl;
    cout << "sizeof(Seq) = " << sizeof(Seq) << endl;

    cout << "test complete" << endl;
}


void seq_test()
{
    // Fixed_seq<Dummy, 50> s_seq;
    // Seq<Dummy> d_seq;

    // // Fixed_seq<int, 5> s_seq;
    // // Seq<int> d_seq;

    // seq_generic_test(s_seq);
    // seq_generic_test(d_seq);

    // cout << "size = " << sizeof(Fixed_seq<int, 0>);
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

    free(v);
}



void ptr_reference_test()
{
    // Debug_os os{std::cout};


    // auto sp1 = shared_ptr<Scope>(new Scope());
    // auto sp2 = shared_ptr<Scope>(new Scope());
    // sp2->dynamic = true;

    // Abstx_identifier id{};
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
    cout << "Shared size = " << sizeof(std::shared_ptr<int>) << endl;
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



void abstx_test()
{
    std::cout << "creating function type" << std::endl;
    CB_Function fn_type; // owned by some global list of types
    fn_type.in_types.add(CB_i8::type);
    fn_type.in_types.add(CB_Range::type);
    fn_type.out_types.add(CB_Float::type);
    fn_type.out_types.add(CB_Range::type);
    fn_type.finalize();


    std::cout << "creating abstx function id" << std::endl;
    Abstx_identifier fn_id; // owned by the scope it is defined in
    fn_id.name = "foo";
    fn_id.cb_type = &fn_type;
    fn_id.finalize();

    std::cout << "creating abstx function" << std::endl;
    Abstx_function fn; // owned by the scope it is defined in
    fn.function_identifier = &fn_id;
    fn.scope = alloc(Abstx_function_scope());
    fn.scope->owner = &fn;
    std::cout << "adding identifiers to function scope" << std::endl;
    fn.scope->add_identifier("in1", CB_i8::type);
    fn.scope->add_identifier("in2", CB_Range::type);
    fn.scope->add_identifier("out1", CB_Float::type);
    fn.scope->add_identifier("out2", CB_Range::type);
    std::cout << "adding arguments from scope to function" << std::endl;
    fn.add_arg(true, fn.scope->get_identifier("in1"));
    fn.add_arg(true, fn.scope->get_identifier("in2"));
    fn.add_arg(false, fn.scope->get_identifier("out1"));
    fn.add_arg(false, fn.scope->get_identifier("out2"));
    std::cout << "finalizing function" << std::endl;
    Parsing_status ps = fn.finalize();
    if (is_codegen_ready(ps)) {
        std::cout << "generating code" << std::endl;
        CB_i8::type->generate_typedef(std::cout);
        CB_Range::type->generate_typedef(std::cout);
        CB_Float::type->generate_typedef(std::cout);
        fn.generate_code(std::cout);
    } else {
        std::cout << "failed to finalize: " << ps << std::endl;
    }
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
    // owning_test();
    // template_test();
    // range_test();
    // flag_test();
    // compile_test();
    // code_gen_test();

    abstx_test();

    std::cout << "all test done!" << std::endl;
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
