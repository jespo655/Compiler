#pragma once

#include "assert.h"

#include <string>
#include <sstream>
#include <cstring>


const int DEFAULT_CAPACITY = 16; // arbitrary power of 2

template<typename T>
struct Seq {
    uint32_t size = 0;
    uint32_t capacity = DEFAULT_CAPACITY;
    T* v_ptr = nullptr;

    std::string toS() const {
        std::ostringstream oss;
        oss << "[";
        for (uint32_t i = 0; i < size; ++i) {
            if (i != 0) oss << ", ";
            oss << v_ptr[i].toS();
        }
        oss << "]";
        return oss.str();
    }

    Seq() {
        size = 0;
        capacity = DEFAULT_CAPACITY;
        v_ptr = (T*)malloc(capacity*sizeof(T));
        // size is always 0 at init, so no need to init members
    }

    ~Seq() {
        for (uint32_t i = 0; i < size; ++i) {
            v_ptr[i].~T();
        }
        free(v_ptr);
        v_ptr = nullptr;
        size = 0;
        capacity = 0;
    }

    // copy
    Seq& operator=(const Seq& Seq) {
        for (uint32_t i = 0; i < size; ++i) {
            v_ptr[i].~T();
        }
        resize(Seq.size, false);
        for (uint32_t i = 0; i < size; ++i) {
            new (&v_ptr[i]) T(Seq.v_ptr[i]);
        }
        return *this;
    }
    Seq(const Seq& Seq) { *this = Seq; }

    // move
    Seq& operator=(Seq&& Seq) {
        this->~Seq(); // free all members
        size = Seq.size;
        capacity = Seq.capacity;
        v_ptr = Seq.v_ptr;

        Seq.size = 0;
        Seq.capacity = 0;
        Seq.v_ptr = nullptr;
        return *this;
    }
    Seq(Seq&& Seq) { *this = std::move(Seq); }


    T& operator[](uint32_t index) {
        if (index >= size) set(index, T());
        return v_ptr[index];
    }
    const T& operator[](uint32_t index) const {
        ASSERT(index < size);
        return v_ptr[index];
    }


    template<typename T2> operator==(const Seq<T2>& Seq) const { return false; }
    bool operator==(const Seq<T>& Seq) const {
        if (size != Seq.size) return false;
        for (int i = 0; i < size; ++i) {
            if (v_ptr[i] != Seq.v_ptr[i]) return false;
        }
        return true;
    }

    T get(uint32_t index) const {
        if (index >= size) return T();
        return v_ptr[index];
    }

    void set(uint32_t index, const T& t) {
        ASSERT(v_ptr != nullptr);
        ASSERT(capacity != 0);
        if (index < size)
            v_ptr[index] = t;
        else {
            while (index >= capacity) {
                reallocate(2*capacity);
            }
            for (uint32_t i = size; i < index; ++i) {
                new (&v_ptr[i]) T(); // fill with default values
            }
            new (&v_ptr[index]) T(t);
            size = index+1;
        }
    }

    void set(uint32_t index, T&& t) {
        ASSERT(v_ptr != nullptr);
        ASSERT(capacity != 0);
        if (index < size)
            v_ptr[index] = t;
        else {
            while (index >= capacity) {
                reallocate(2*capacity);
            }
            for (uint32_t i = size; i < index; ++i) {
                new (&v_ptr[i]) T(); // fill with default values
            }
            new (&v_ptr[index]) T(std::move(t));
            size = index+1;
        }
    }

    bool empty() const {
        return size == 0;
    }

    void add(const T& t) {
        set(size, t);
    }

    void add(T&& t) {
        set(size, std::move(t));
    }

    void resize(uint32_t new_size, bool init=true, T default_value=T()) {
        if (new_size > size) {
            if (new_size > capacity) {
                reallocate(new_size);
            }
            if (init) {
                for (uint32_t i = size; i < new_size; ++i) {
                    new (&v_ptr[i]) T(default_value); // fill with default values
                }
            }
        } else {
            // properly delete everything outside the new size
            for (uint32_t i = new_size; i < size; ++i) v_ptr[i].~T();
        }
        size = new_size;
    }

    void clear() {
        // properly delete cleared things
        for (uint32_t i = 0; i < size; ++i)
            v_ptr[i].~T();
        size = 0;
    }

    void reallocate(uint32_t capacity, bool move=true) {
        if (this->capacity == capacity) return;
        this->capacity = capacity;
        T* new_ptr = (T*)malloc(capacity*sizeof(T));
        ASSERT(new_ptr != nullptr);
        if (capacity < size) {
            for (uint32_t i = capacity; i < size; ++i) v_ptr[i].~T(); // properly delete the Ts we won't move
            size = capacity;
        }
        if (move) memcpy(new_ptr, v_ptr, size*sizeof(T)); // move the values
        else for (uint32_t i = 0; i < size; ++i) v_ptr[i].~T(); // properly delete all Ts we don't move
        free(v_ptr);
        v_ptr = new_ptr;
    }

    struct iterator {
        uint32_t index = 0;
        Seq& _seq;
        iterator(Seq& _seq, uint32_t i=0) : _seq{_seq}, index{i} {}
        T& operator*() const { ASSERT(index < _seq.size); return _seq.v_ptr[index]; }
        iterator& operator++() { ++index; return *this; }
        bool operator==(const iterator& o) const { return index == o.index; }
        bool operator!=(const iterator& o) const { return !(*this == o); }
        bool operator<(const iterator& o) const { return index < o.index; }
    };

    struct const_iterator {
        uint32_t index = 0;
        const Seq& _seq;
        const_iterator(const Seq& _seq, uint32_t i=0) : _seq{_seq}, index{i} {}
        const T& operator*() const { ASSERT(index < _seq.size); return _seq.v_ptr[index]; }
        const_iterator& operator++() { ++index; return *this; }
        bool operator==(const const_iterator& o) const { return index == o.index; }
        bool operator!=(const const_iterator& o) const { return !(*this == o); }
        bool operator<(const const_iterator& o) const { return index < o.index; }
    };

    iterator begin() { return iterator(*this); }
    iterator end() { return iterator(*this, size); }

    const_iterator begin() const { return const_iterator(*this); }
    const_iterator end() const { return const_iterator(*this, size); }
};




/*
Static sequence - stores the elements on the stack

Syntax:
a : T[N] = [T, size=N: t1, t2, t3]; // T inferred from members, N evaluated compile time, must be a positive integer.
a : T[N] = [T: t1, t2, t3]; // N inferred by the number of arguments
a : T[N] = [size=N: t1, t2, t3]; // T inferred from the type of the members
a : T[N] = [t1, t2, t3]; // T and N inferred
*/

template<typename T, uint32_t c_size>
struct Fixed_seq {
    constexpr static uint32_t size = c_size;
    constexpr static uint32_t capacity = c_size;
    T v[c_size];

    std::string toS() const {
        std::ostringstream oss;
        oss << "[size=" << size << ": ";
        for (uint32_t i = 0; i < size; ++i) {
            if (i != 0) oss << ", ";
            oss << v[i].toS();
        }
        oss << "]";
        return oss.str();
    }

    Fixed_seq(bool init=true) {
        if (init) {
            for (uint32_t i = 0; i < size; ++i) {
                new (&v[i]) T();
            }
        }
    }
    ~Fixed_seq() {
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

    explicit operator Seq<T>() const
    {
        Seq<T> Seq;
        Seq.reallocate(size);
        Seq.size = size;
        memcpy(Seq.v_ptr, v, size*sizeof(T));
        return Seq;
    }

    struct iterator {
        uint32_t index = 0;
        const Fixed_seq& Seq;
        iterator(const Fixed_seq& Seq, uint32_t i=0) : Seq{Seq}, index{i} {}
        const T& operator*() const { ASSERT(index < Seq.size); return Seq.v_ptr[index]; }
        iterator& operator++() { ++index; return *this; }
        bool operator==(const iterator& o) const { return index == o.index; }
        bool operator!=(const iterator& o) const { return !(*this == o); }
        bool operator<(const iterator& o) const { return index < o.index; }
    };

    iterator begin() const { return iterator(*this); }
    iterator end() const { return iterator(*this, size); }
};








/*
// Problem: Partial template of functions doesn't work @check

// copy
template<typename T>
Seq<T>& Seq<T>::operator=(const Seq<T>& Seq) {
    for (uint32_t i = 0; i < size; ++i) {
        v_ptr[i].~T();
    }
    resize(Seq.size, false);
    for (uint32_t i = 0; i < size; ++i) {
        new (&v_ptr[i]) T(Seq.v_ptr[i]);
    }
    return *this;
}

#include "pointers.h"
template<typename T, typename PT=CB_Owning_pointer<T>>
Seq<PT>& Seq<PT>::operator=(const Seq<PT>& Seq) {
    for (uint32_t i = 0; i < size; ++i) {
        v_ptr[i].~T();
    }
    resize(Seq.size, false);
    for (uint32_t i = 0; i < size; ++i) {
        // new (&v_ptr[i]) T(Seq.v_ptr[i]);
    }
    return *this;
}


*/






