#pragma once

#include "type.h"
#include "dynamic_seq.h" // cast operator

#include <string>
#include <sstream>

/*
Static sequence - stores the elements on the stack

Syntax:
a : T[N] = [T, size=N: t1, t2, t3]; // T inferred from members, N evaluated compile time, must be a positive integer.
a : T[N] = [T: t1, t2, t3]; // N inferred by the number of arguments
a : T[N] = [size=N: t1, t2, t3]; // T inferred from the type of the members
a : T[N] = [t1, t2, t3]; // T and N inferred
*/

template<typename T, uint32_t c_size> // T is a CB type
struct CB_Static_seq {
    static CB_Type type;
    static const bool primitive = false;
    constexpr static CB_Type& member_type = T::type;
    constexpr static uint32_t size = c_size;
    constexpr static uint32_t capacity = c_size;
    T v[c_size];

    std::string toS() const {
        std::ostringstream oss;
        oss << "[" << member_type.toS() << ", size=" << size << ": ";
        for (uint32_t i = 0; i < size; ++i) {
            if (i != 0) oss << ", ";
            oss << v[i].toS();
        }
        oss << "]";
        return oss.str();
    }

    CB_Static_seq(bool init=true) {
        if (init) {
            for (uint32_t i = 0; i < size; ++i) {
                new (&v[i]) T();
            }
        }
    }
    ~CB_Static_seq() {
        for (uint32_t i = 0; i < size; ++i) v[i].~T();
    }

    T& operator[](uint32_t index) {
        ASSERT(index < c_size);
        return v[index];
    }
    T operator[](uint32_t index) const { return get(index); }

    T get(uint32_t index) const {
        if (index >= c_size) return T();
        return v[index];
    }

    void set(uint32_t index, T t) {
        if (index < c_size) {
            v[index] = t;
        } else {
            // outside the array -> ignore
        }
    }

    operator CB_Dynamic_seq<T>() const
    {
        CB_Dynamic_seq<T> seq;
        seq.reallocate(size);
        seq.size = size;
        memcpy(seq.v_ptr, v, size*sizeof(T));
        return seq;
    }

    struct iterator {
        uint32_t index = 0;
        const CB_Static_seq& seq;
        iterator(const CB_Static_seq& seq, uint32_t i=0) : seq{seq}, index{i} {}
        const T& operator*() const { ASSERT(index < seq.size); return seq.v_ptr[index]; }
        iterator& operator++() { ++index; return *this; }
        bool operator==(const iterator& o) const { return index == o.index; }
        bool operator!=(const iterator& o) const { return !(*this == o); }
        bool operator<(const iterator& o) const { return index < o.index; }
    };

    iterator begin() const { return iterator(*this); }
    iterator end() const { return iterator(*this, size); }
};

template<typename T, uint32_t size>
CB_Type CB_Static_seq<T, size>::type = CB_Type("["+std::to_string(size)+"]"+T::type.toS(), sizeof(CB_Static_seq<T, size>), CB_Static_seq<T,size>());

