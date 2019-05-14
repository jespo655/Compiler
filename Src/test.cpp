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
#include "abstx/all_abstx.h"
#include "types/all_cb_types.h"
#include "utilities/assert.h"
#include "utilities/unique_id.h"
#include "parser/parser.h"
#include "lexer/lexer.h"
#include "tests/unit_tests.h"
#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <memory>
using namespace std;











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
    int i = get_unique_id();
    bool deleted = false;
    Verbose() { cout << "V " << std::dec << i << " constructed" << endl; }
    ~Verbose() {
        cout << "V " << std::dec << i << " deleted";
        if (deleted) cout << " FOR THE SECOND TIME @@@@@@@@@@@@@@@@@@@@@" << endl;
        cout << endl;
        deleted = true;
    }
    Verbose(const Verbose& v) { cout << "V " << std::dec << i << " copy constructor from " << v.i << endl; }
    Verbose& operator=(const Verbose& v) { cout << "V " << std::dec << i << " copy assignment from " << v.i << endl; return *this; }
    Verbose(Verbose&& v) { cout << "V " << std::dec << i << " move constructor from " << v.i << endl; }
    Verbose& operator=(Verbose&& v) { cout << "V " << std::dec << i << " move assignment from " << v.i << endl; return *this; }
};


Owned<Verbose> create_verbose()
{
    cout << "creating" << endl;
    Owned<Verbose> vp = alloc(Verbose());
    cout << "returning" << endl;
    return vp;
}

void grab_verbose(Owned<Verbose> vp) {
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


Seq<Verbose> get_verbose_seq() {
    std::cout << "creating seq" << std::endl;
    Seq<Verbose> seq;
    seq.add(Verbose());
    seq[17] = Verbose();
    std::cout << "returning seq" << std::endl;
    return seq;
}

void sequence_test()
{
    Seq<Verbose> seq = get_verbose_seq();
    seq.add(Verbose());
    for (const Verbose& v : seq) std::cout << std::dec << v.i << " ";
    std::cout << std::endl;
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






#ifdef NOT_DEFINED

// not very useful test location since almost all things we would want to test here requires other cpp files (e.g. types)

#include "../types/all_cb_types.h"
#include <iostream>

int main()
{
    { Abstx_assignment abstx; }
    { Abstx_declaration abstx; }
    { Abstx_defer abstx; }
    // { Abstx_for abstx; }
    { Abstx_function_literal abstx; }
    { Abstx_function_call abstx; }
    // { Abstx_if abstx; } // undefined reference to bool
    { Abstx_return abstx; }
    // { Abstx_scope abstx; } // log_error undefined
    // { Statement abstx; } // abstract class
    { Abstx_using abstx; }
    // { Abstx_while abstx; } // undefined reference to bool
}

#endif







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

    cout << "unique size = " << sizeof(Owned<int>) << endl;
    cout << "Shared size = " << sizeof(Shared<int>) << endl;
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
    /*
    // Shared<Global_scope> gs = parse_file("../Demos/helloworld.cb");
    Seq<Token> hw_tokens = get_tokens_from_file("../Demos/helloworld.cb");
    std::cout << "../Demos/helloworld.cb tokens: " << endl;
    for (const Token& token : hw_tokens) {
        if (token.type == Token_type::STRING) std::cout << "\"" << token.token << "\" ";
        else std::cout << token.token << " ";
    }
    std::cout << endl;

    Seq<Token> code_tokens = get_tokens_from_file("lexer/code.cb");
    std::cout << "lexer/code.cb tokens: " << endl;
    for (const Token& token : code_tokens) {
        if (token.type == Token_type::STRING) std::cout << "\"" << token.token << "\" ";
        else std::cout << token.token << " ";
    }
    std::cout << endl;
    */

    Seq<Token> code_tokens = get_tokens_from_file("test.cb");
    std::cout << "test.cb tokens: " << endl;
    for (const Token& token : code_tokens) {
        if (token.type == Token_type::STRING) std::cout << "\"" << token.token << "\" ";
        else std::cout << token.token << " ";
    }
    std::cout << endl;

    std::cout << "done!" << std::endl;

}

void code_gen_test()
{
    // first: get tokens from file
    // Seq<Token> hw_tokens = get_tokens_from_file("../Demos/minimal.cb");
    // std::cout << "../Demos/minimal.cb tokens: " << endl;
    // for (const Token& token : hw_tokens) {
    //     // if (token.type == Token_type::STRING) std::cout << "\"" << token.token << "\" ";
    //     // else std::cout << token.token << " ";
    //     std::cout << token << ", ";
    // }
    // std::cout << endl;

    Shared<Global_scope> gs = parse_file("../Demos/minimal.cb");
    // Shared<Global_scope> gs = parse_tokens(std::move(hw_tokens), "Demos/minimal.cb");

    // later:
    // find identifier main, it should be a function
    // fully parse starting from that function
    // then generate code starting from that function

    LOG("exiting if errors");
    exit_if_errors();

    auto ep = gs->get_entry_point("bar");
    ep->finalize();


    // std::map<std::string, Shared<Abstx_identifier>> identifiers
    // for (auto& p : gs->identifiers) {
    //     LOG(p.first << ": " << p.second->toS());
    // }



    // LOG("generating typedefs");
    // generate_typedefs(std::cout);

    LOG("generating statement code");
    for (const auto& s : gs->statements) {
        ASSERT(s); // no statement can be nullpointer here
        // @TODO: any dependencies should be generated FIRST!
        s->generate_code(std::cout); // for now, just fully parse the statements
    }

    LOG("generating used function code");
    for (const auto& fn : gs->used_functions) {
        ASSERT(fn.second); // no f unction can be nullpointer here
        fn.second->generate_declaration(std::cout, std::cout);
    }

    // compile into abstx tree
    // TODO

    // get entry point
    // Function entry_point;
    // std::set<std::string> c_sources;
    // generate_code(std::cout, &fn, c_sources);
}







/*


void test_type(Shared<const CB_Type> type)
{
    std::cout << type->toS() << ": ";
    type->generate_type(std::cout);
    std::cout << ", uid: " << type->uid << ", size: " << type->cb_sizeof()
        << ", defval: ";
    type->generate_literal(std::cout, type->default_value().v_ptr);
    std::cout << std::endl;
}

Seq<Owned<Abstx_identifier>> identifiers;

Shared<Abstx_identifier> make_member(const std::string& name, Shared<const CB_Type> type)
{
    Owned<Abstx_identifier> o = alloc(Abstx_identifier());
    o->name = name;
    o->value.v_type = type;
    Shared<Abstx_identifier> s = o;
    identifiers.add(std::move(o));
    return s;
}

int types_test()
{
    test_type(CB_Type::type);
    test_type(CB_i8::type);
    test_type(CB_i16::type);
    test_type(CB_i32::type);
    test_type(CB_i64::type);
    test_type(CB_Int::type);
    test_type(CB_u8::type);
    test_type(CB_u16::type);
    test_type(CB_u32::type);
    test_type(CB_u64::type);
    test_type(CB_Uint::type);
    test_type(CB_f32::type);
    test_type(CB_f64::type);
    test_type(CB_Float::type);
    test_type(CB_Bool::type);
    test_type(CB_Flag::type);
    test_type(CB_Range::type);
    test_type(CB_Float_range::type);
    test_type(CB_String::type);

    { CB_String type; test_type(&type); }
    { CB_u8 type; test_type(&type); }
    { CB_Range type; test_type(&type); }
    { CB_Flag type; test_type(&type); }
    { CB_Flag type; test_type(&type); }

    // These fails assert (function literals cannot be generated)
    // { CB_Function type; type.finalize(); test_type(&type); }
    // { CB_Function type; type.finalize(); test_type(&type); }
    // { CB_Function type; type.in_types.add(CB_Bool::type); type.finalize(); test_type(&type); }
    // { CB_Function type; type.out_types.add(CB_Bool::type); type.finalize(); test_type(&type); }

    { CB_Struct type; type.add_member(make_member("i", CB_Int::type)); type.finalize(); test_type(&type); }
    { CB_Struct type; type.add_member(make_member("i", CB_Int::type)); type.finalize(); test_type(&type); }
    { CB_Struct type; type.add_member(make_member("i", CB_Int::type)); type.finalize(); test_type(&type); }
    { CB_Struct type; type.add_member(make_member("s", CB_String::type)); type.finalize(); test_type(&type); }

    {
        CB_Struct type; type.add_member(make_member("s", CB_String::type)); type.finalize();
        CB_Struct type2 = type;
        test_type(&type);
        test_type(&type2);
        CB_Struct type3 = std::move(type);
        test_type(&type3);
        // test_type(&type);
    }

    {
        CB_Struct type;
        type.add_member(make_member("a", CB_i8::type));
        type.add_member(make_member("b", CB_i16::type));
        type.add_member(make_member("c", CB_i16::type));
        type.add_member(make_member("d", CB_i16::type));
        type.finalize();
        test_type(&type);
    }

    { CB_Pointer type; type.v_type = CB_Int::type; type.finalize(); test_type(&type); }
    { CB_Pointer type; type.v_type = CB_String::type; type.finalize(); test_type(&type); }
    { CB_Pointer type; type.v_type = CB_Range::type; type.finalize(); test_type(&type); }
    {
        CB_Pointer type; type.v_type = CB_Range::type; type.finalize(); test_type(&type);
        CB_Pointer pt1; pt1.v_type = &type; pt1.owning=true; pt1.finalize(); test_type(&pt1);
        CB_Pointer pt2; pt2.v_type = &pt1;  pt2.owning=false; pt2.finalize(); test_type(&pt2);
        CB_Pointer pt3; pt3.v_type = &pt2;  pt3.owning=true; pt3.finalize(); test_type(&pt3);
        CB_Pointer pt4; pt4.v_type = &pt3;  pt4.owning=true; pt4.finalize(); test_type(&pt4);
        CB_Pointer pt5; pt5.v_type = &pt4;  pt5.owning=true; pt5.finalize(); test_type(&pt5);
        pt5.generate_destructor(std::cout, "ptr");
    }

    {
        CB_Struct type;
        CB_Pointer pt1; pt1.finalize(); // forward declaration / finalize as unresolved_pointer
        // CB_Pointer pt1; pt1.v_type = CB_Int::type; pt1.owning=false; pt1.finalize();
        CB_Pointer pt2; pt2.v_type = &pt1; pt2.owning=true; pt2.finalize();
        type.add_member(make_member("sp", &pt1));
        type.add_member(make_member("op", &pt2));
        type.finalize();
        pt1.v_type = &type; pt1.owning=false; pt1.finalize(); // final finalize now when type is finalized
        test_type(&type);
        test_type(&pt1);
        type.generate_destructor(std::cout, "s");
    }

    {
        CB_Struct s1, s2, s3;
        CB_Pointer pt1; pt1.finalize(); // forward declaration / finalize as unresolved_pointer
        s1.add_member(make_member("s3op", &pt1)); s1.finalize();
        s2.add_member(make_member("s1", &s1)); s2.finalize();
        s3.add_member(make_member("s2", &s2)); s3.finalize();
        pt1.v_type = &s3; pt1.owning=true; pt1.finalize(); // final finalize now when type is finalized
        // s1.generate_literal(std::cout, s1._default_value); std::cout << std::endl;
        // s2.generate_destructor(std::cout, "s"); std::cout << std::endl; // this should give cyclic reference error
    }

    // @TODO: (CB_Set)

    { CB_Seq s; s.v_type = CB_Int::type; s.finalize(); test_type(&s); }
    { CB_Fixed_seq s; s.v_type = CB_Int::type; s.size = 5; s.finalize(); test_type(&s); }
    { CB_Fixed_seq s; s.v_type = CB_Int::type; s.size = 5; s.finalize(); test_type(&s); }
    { CB_Fixed_seq s; s.v_type = CB_Int::type; s.size = 6; s.finalize(); test_type(&s); }
    { CB_Fixed_seq s; s.v_type = CB_i64::type; s.size = 5; s.finalize(); test_type(&s); }

}


*/






void abstx_test()
{
    // std::cout << "creating function type" << std::endl;
    // Owned<CB_Function> owned_fn_type = alloc(CB_Function()); // owned by some global list of types
    // ASSERT(owned_fn_type);
    // owned_fn_type->in_types.add(get_built_in_type("i8"));
    // owned_fn_type->in_types.add(get_built_in_type("range"));
    // owned_fn_type->out_types.add(get_built_in_type("float"));
    // owned_fn_type->out_types.add(get_built_in_type("range"));
    // owned_fn_type->finalize();
    // ASSERT(owned_fn_type);
    // std::cout << "adding complex type" << std::endl;
    // Shared<const CB_Function> fn_type = add_complex_type(std::move(owned_fn_type));
    // ASSERT(fn_type);

    // std::cout << "creating abstx function id" << std::endl;
    // Abstx_identifier fn_id; // owned by the scope it is defined in
    // fn_id.name = "foo";
    // fn_id.value.v_type = static_pointer_cast<const CB_Type>(fn_type);
    // // fn_id.finalize();

    // std::cout << "creating abstx function" << std::endl;
    // Abstx_function_literal fn; // owned by the scope it is defined in
    // fn.function_identifier = &fn_id;
    // fn.scope = alloc(Abstx_function_scope());
    // fn.scope->owner = &fn;
    // std::cout << "adding identifiers to function scope" << std::endl;
    // fn.scope->add_identifier("in1", fn_type->in_types[0]);
    // fn.scope->add_identifier("in2", fn_type->in_types[1]);
    // fn.scope->add_identifier("out1", fn_type->out_types[0]);
    // fn.scope->add_identifier("out2", fn_type->out_types[1]);
    // std::cout << "adding arguments from scope to function" << std::endl;
    // fn.add_arg(true, fn.scope->get_identifier("in1"));
    // fn.add_arg(true, fn.scope->get_identifier("in2"));
    // fn.add_arg(false, fn.scope->get_identifier("out1"));
    // fn.add_arg(false, fn.scope->get_identifier("out2"));
    // std::cout << "finalizing function" << std::endl;
    // // Parsing_status ps = fn.finalize();
    // if (is_codegen_ready(fn.status)) {
    //     std::cout << "generating code" << std::endl;
    //     generate_typedefs(std::cout);
    //     fn.generate_code(std::cout);
    // } else {
    //     std::cout << "failed to finalize: " << fn.status << std::endl;
    // }
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
    // abstx_test();

    // compile_test();
    // sequence_test();
    // code_gen_test();

    // test_types();
    run_default_test_suites();
    // std::cout << "all test done!" << std::endl;
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
