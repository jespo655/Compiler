#ifdef TEST

#include "unique_id.h"
#include "pointers.h"
#include "sequence.h"
// #include "static_any.h"
// #include "../types/type.h"
// #include "../types/function.h"
#include <iostream>
#include <string>
#include <iomanip>




struct S {
    int i = 2;
    virtual std::string toS() const { return "S";}
    void foo() { static int i = 0; std::cout << ++i << std::endl; }
};

struct S2 : S {
    int stat = 2;
    std::string toS() const override { return "S2";}
};

struct Q { int j; };

template<typename T>
struct TS
{
    constexpr static int stat = 3;
    T t;
    std::string toS() const { return "TS";}
};


Shared<const S> super_foo(Owned<S>&& type) {
    std::cout << "    type: " << std::hex << type.v << std::endl;
    Owned<S> o = std::move(type);
    std::cout << "    !type: " << std::hex << type.v << std::endl;
    std::cout << "    o: " << std::hex << o.v << std::endl;
    return (Shared<S>)o; // double cast
}

template<typename T>
Shared<const T> template_foo(Owned<T>&& type) {
    std::cout << "  type: " << std::hex << type.v << std::endl;
    Owned<S> o = owned_static_cast<S>(std::move(type));
    std::cout << "  o: " << std::hex << o.v << std::endl;
    Shared<const S> p = super_foo(std::move(o));
    std::cout << "  p: " << std::hex << p.v << std::endl;
    Shared<const T> p2 = static_pointer_cast<const T>(p);
    std::cout << "  p2: " << std::hex << p2.v << std::endl;
    return p2;
}

void pointer_test()
{
    Owned<S2> o = alloc(S2());
    std::cout << std::hex << o.v << std::endl;
    ASSERT(o);
    std::cout << "o: " << std::hex << o.v << std::endl;
    Shared<const S2> s = template_foo(std::move(o));
    ASSERT(s);
    ASSERT(!o);
    std::cout << "s: " << std::hex << s.v << std::endl;
    std::cout << "!o: " << std::hex << o.v << std::endl;
    // std::cout << s->toS() << std::endl;
}



void static_test()
{
    S s1, s2;
    s1.foo();
    s1.foo();
    s1.foo();
    s1.foo();
    s2.foo();
    s2.foo();
    s2.foo();
    s2.foo();
}


Seq<int> get_seq() {
    std::cout << "constructing seq in fn" << std::endl;
    Seq<int> seq;
    for (int i = 50; i < 60; ++i) seq.add(i);
    std::cout << "returning seq in fn" << std::endl;
    return seq;
}

void sequence_test()
{
    std::cout << "constructing a" << std::endl;
    Seq<int> a = get_seq();
    std::cout << "adding to a" << std::endl;
    for (int i = 0; i < 10; ++i) a.add(i);

    std::cout << "constructing b (move from a)" << std::endl;
    Seq<int> b = std::move(a);
    std::cout << "adding to b" << std::endl;
    for (int i = 10; i < 20; ++i) b.add(i);

    std::cout << "printing b" << std::endl;
    for (int i : b) std::cout << i << " ";
    std::cout << std::endl;

    // for (const int& i : b) std::cout << i << " ";
    // std::cout << std::endl;
    std::cout << "doing crazy stuff!" << std::endl;
    b = std::move(b);

    std::cout << "deleting!" << std::endl;
    a.~Seq();
    b.~Seq();
    std::cout << "done!" << std::endl;
}

int main()
{
    sequence_test();
    // static_test();
    // pointer_test();

    // Shared<void> s;
    // void* v = s.v;
    // Owned<void> o;

    // S s;
    // S2 s2;

    // Shared<S> sp = &s;
    // Shared<S> sp2 = &s2;

    // Shared<S> sp3 = sp2;

    // std::cout << sp->toS() << std::endl;
    // std::cout << sp2->toS() << std::endl;
    // std::cout << sp3->toS() << std::endl;



    // TS<S> t;
    // TS<S2> t2;

    // std::cout << "t: " << t.t.i << std::endl;
    // std::cout << "t2: " << t2.t.i << std::endl;

    // Seq<S> s;
    // Seq<TS<S>> ts;

    // s.resize(3, true);
    // ts.resize(3, false);

    // std::cout << "s: " << s.toS() << std::endl;
    // std::cout << "ts: " << ts.toS() << std::endl;

    // Shared<Seq<S>> ps = &s;
    // Owned<Seq<TS<S>>> pts = &ts;

    // std::cout << "ps: " << ps.toS() << std::endl;
    // std::cout << "pts: " << pts.toS() << std::endl;

    // std::cout << -2UL << std::endl;

    // std::cout << sizeof(void(*)()) << std::endl;
    // std::cout << sizeof(void*) << std::endl;

    // bool bt = true;
    // bool bf = false;

    // std::cout << "bt: " << bt << ", bf: " << bf << std::endl;

    // CB_Function f1, f2;
    // std::cout << "1: " << f1.toS() << ", " << f1.uid << "; " << f2.toS() << ", " << f2.uid << std::endl;
    // f1.finalize();
    // f2.finalize();
    // std::cout << "1: " << f1.toS() << ", " << f1.uid << "; " << f2.toS() << ", " << f2.uid << std::endl;

}


#endif
