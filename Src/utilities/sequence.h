#pragma once

#include "assert.h"

#include <string>
#include <sstream>
#include <cstring>

/*
Dynamic sequence - stores the elements on the heap

Syntax:
a : T[] = [T, size=N: t1, t2, t3]; // T inferred from members, N evaluated compile time, must be a positive integer.
a : T[] = [T: t1, t2, t3]; // N inferred by the number of arguments
a : T[] = [size=N: t1, t2, t3]; // T inferred from the type of the members
a : T[] = [t1, t2, t3]; // T and N inferred
*/

const int DEFAULT_CAPACITY = 16; // arbitrary power of 2

template<typename T>
struct seq {
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

    seq() {
        size = 0;
        capacity = DEFAULT_CAPACITY;
        v_ptr = (T*)malloc(capacity*sizeof(T));
        // size is always 0 at init, so no need to init members
    }

    ~seq() {
        for (uint32_t i = 0; i < size; ++i) {
            v_ptr[i].~T();
        }
        free(v_ptr);
        v_ptr = nullptr;
        size = 0;
        capacity = 0;
    }

    // copy
    seq& operator=(const seq& seq) {
        for (uint32_t i = 0; i < size; ++i) {
            v_ptr[i].~T();
        }
        resize(seq.size, false);
        for (uint32_t i = 0; i < size; ++i) {
            new (&v_ptr[i]) T(seq.v_ptr[i]);
        }
        return *this;
    }
    seq(const seq& seq) { *this = seq; }

    // move
    seq& operator=(seq&& seq) {
        this->~seq(); // free all members
        size = seq.size;
        capacity = seq.capacity;
        v_ptr = seq.v_ptr;

        seq.size = 0;
        seq.capacity = 0;
        seq.v_ptr = nullptr;
        return *this;
    }
    seq(seq&& seq) { *this = std::move(seq); }


    T& operator[](uint32_t index) {
        if (index >= size) set(index, T());
        return v_ptr[index];
    }
    T operator[](uint32_t index) const { return get(index); }


    template<typename T2> operator==(const seq<T2>& seq) const { return false; }
    bool operator==(const seq<T>& seq) const {
        if (size != seq.size) return false;
        for (int i = 0; i < size; ++i) {
            if (v_ptr[i] != seq.v_ptr[i]) return false;
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

    bool empty() const {
        return size == 0;
    }

    void add(const T& t) {
        set(size, t);
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
        seq& _seq;
        iterator(seq& _seq, uint32_t i=0) : _seq{_seq}, index{i} {}
        T& operator*() const { ASSERT(index < _seq.size); return _seq.v_ptr[index]; }
        iterator& operator++() { ++index; return *this; }
        bool operator==(const iterator& o) const { return index == o.index; }
        bool operator!=(const iterator& o) const { return !(*this == o); }
        bool operator<(const iterator& o) const { return index < o.index; }
    };

    struct const_iterator {
        uint32_t index = 0;
        const seq& _seq;
        const_iterator(const seq& _seq, uint32_t i=0) : _seq{_seq}, index{i} {}
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
struct fixed_seq {
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

    fixed_seq(bool init=true) {
        if (init) {
            for (uint32_t i = 0; i < size; ++i) {
                new (&v[i]) T();
            }
        }
    }
    ~fixed_seq() {
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

    explicit operator seq<T>() const
    {
        seq<T> seq;
        seq.reallocate(size);
        seq.size = size;
        memcpy(seq.v_ptr, v, size*sizeof(T));
        return seq;
    }

    struct iterator {
        uint32_t index = 0;
        const fixed_seq& seq;
        iterator(const fixed_seq& seq, uint32_t i=0) : seq{seq}, index{i} {}
        const T& operator*() const { ASSERT(index < seq.size); return seq.v_ptr[index]; }
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
seq<T>& seq<T>::operator=(const seq<T>& seq) {
    for (uint32_t i = 0; i < size; ++i) {
        v_ptr[i].~T();
    }
    resize(seq.size, false);
    for (uint32_t i = 0; i < size; ++i) {
        new (&v_ptr[i]) T(seq.v_ptr[i]);
    }
    return *this;
}

#include "pointers.h"
template<typename T, typename PT=CB_Owning_pointer<T>>
seq<PT>& seq<PT>::operator=(const seq<PT>& seq) {
    for (uint32_t i = 0; i < size; ++i) {
        v_ptr[i].~T();
    }
    resize(seq.size, false);
    for (uint32_t i = 0; i < size; ++i) {
        // new (&v_ptr[i]) T(seq.v_ptr[i]);
    }
    return *this;
}


*/






