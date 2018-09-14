#pragma once

#include <cstring>



/*
TODO:

<< / >> operator:
läs utf8 char från istream

*/



template<int size>
struct str {

    char v[size];

    str() {}
    str& operator=(const char* c)
    {
        bool end = false;
        for (int i = 0; i < size; ++i) {
            if (end) v[i] = 0;
            else v[i] = c[i];
            if (c[i] == '\0') end = true;
        }
        return *this;
    }

};


template<int S1, int S2>
str<S1+S2> operator+(const str<S1>& lhs, const str<S2>& rhs)
{
    str<S1+S2> r;
    int p = 0;
    for (int i = 0; i < S1; ++i) {
        if (lhs[i] == '\0') break;
        r.v[p++] = lhs[i];
    }
    for (int i = 0; i < S2; ++i) {
        r.v[p++] = rhs[i];
    }
    return r;
}


template<int S1, int S2>
bool operator==(const str<S1>& lhs, const str<S2>& rhs)
{
    for (int i = 0; i < S1 && i < S2; ++i) {
        if (lhs[i] != rhs[i]) return false;
        if (lhs[i] == '\0') return true;
    }
    if (S1 > S2) return lhs[S2] == '\0';
    if (S1 < S2) return rhs[S1] == '\0';
    return true;
}

