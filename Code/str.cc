#include "str.h"
#include <iostream>
using namespace std;

int main()
{
    str<5> s1;
    s1 = "as1";

    str<5> s2;
    s2 = "as22345678";

    auto s3 = s1+s2;

    cout << s1.v << endl;
    cout << s2.v << endl;
    cout << s3.v << endl;
}