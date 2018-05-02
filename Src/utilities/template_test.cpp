#ifdef TEST

#include "unique_id.h"
#include "pointers.h"
#include "sequence.h"
// #include "static_any.h"
// #include "../types/type.h"
// #include "../types/function.h"
#include <iostream>
#include <string>


struct S {
    int i = 2;
    virtual std::string toS() { return "S";}
};

struct S2 : S {
    int stat = 2;
    std::string toS() override { return "S2";}
};

struct Q { int j; };

template<typename T>
struct TS
{
    constexpr static int stat = 3;
    T t;
    std::string toS() { return "TS";}
};

int main()
{
    S s;
    S2 s2;

    Shared<S> sp = &s;
    Shared<S> sp2 = &s2;

    Shared<S> sp3 = sp2;

    std::cout << sp->toS() << std::endl;
    std::cout << sp2->toS() << std::endl;
    std::cout << sp3->toS() << std::endl;



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
