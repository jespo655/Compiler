#ifdef TEST

#include "pointers.h"
#include "sequence.h"
#include <iostream>


struct S { int i;
    constexpr static int stat = 1;
    std::string toS() { return "S";}
};

struct S2 : S {
    int stat = 2;
    std::string toS() { return "S2";}
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
    TS<S> t;
    TS<S2> t2;

    std::cout << "t: " << t.t.i << std::endl;
    std::cout << "t2: " << t2.t.i << std::endl;

    std::cout << "t.stat: " << t.t.stat << std::endl;
    std::cout << "t2.stat: " << t2.t.stat << std::endl;
    t2.t.stat = 5;
    std::cout << "t2.stat: " << t2.t.stat << std::endl;

    seq<S> s;
    seq<TS<S>> ts;

    s.resize(3, true);
    ts.resize(3, false);

    std::cout << "s: " << s.toS() << std::endl;
    std::cout << "ts: " << ts.toS() << std::endl;

    shared<seq<S>> ps = &s;
    owned<seq<TS<S>>> pts = &ts;

    std::cout << "ps: " << ps.toS() << std::endl;
    std::cout << "pts: " << pts.toS() << std::endl;

    std::cout << -2UL << std::endl;
}

#endif
