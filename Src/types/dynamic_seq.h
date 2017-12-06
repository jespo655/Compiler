#pragma once

#include "type.h"

#include <string>
#include <sstream>

/*
Dynamic sequence - stores the elements on the heap

Syntax:
a : T[] = [T, size=N: t1, t2, t3]; // T inferred from members, N evaluated compile time, must be a positive integer.
a : T[] = [T: t1, t2, t3]; // N inferred by the number of arguments
a : T[] = [size=N: t1, t2, t3]; // T inferred from the type of the members
a : T[] = [t1, t2, t3]; // T and N inferred
*/

const int DEFAULT_CAPACITY = 16; // arbitrary power of 2

template<typename T> // T is a CB type
struct CB_Dynamic_seq {
    static CB_Type type;
    static const bool primitive = false;
    constexpr static CB_Type& member_type = T::type;
    uint32_t size = 0;
    uint32_t capacity = DEFAULT_CAPACITY;
    T* v_ptr = nullptr;

    std::string toS() const {
        std::ostringstream oss;
        oss << "[" << member_type.toS() << ", dynamic: ";
        for (uint32_t i = 0; i < size; ++i) {
            if (i != 0) oss << ", ";
            oss << v_ptr[i].toS();
        }
        oss << "]";
        return oss.str();
    }

    CB_Dynamic_seq() {
        size = 0;
        capacity = DEFAULT_CAPACITY; // arbitrary power of 2
        v_ptr = (T*)malloc(capacity*sizeof(T));
        // size is always 0 at init, so no need to init members
    }

    ~CB_Dynamic_seq() {
        for (uint32_t i = 0; i < size; ++i) {
            v_ptr[i].~T();
        }
        free(v_ptr);
        v_ptr = nullptr;
        size = 0;
        capacity = 0;
    }

    // copy
    CB_Dynamic_seq& operator=(const CB_Dynamic_seq& seq) {
        for (uint32_t i = 0; i < size; ++i) {
            v_ptr[i].~T();
        }
        resize(seq.size, false);
        for (uint32_t i = 0; i < size; ++i) {
            new (&v_ptr[i]) T(seq.v_ptr[i]);
        }
        return *this;
    }
    CB_Dynamic_seq(const CB_Dynamic_seq& seq) { *this = seq; }

    // move
    CB_Dynamic_seq& operator=(CB_Dynamic_seq&& seq) {
        this->~CB_Dynamic_seq(); // free all members
        size = seq.size;
        capacity = seq.capacity;
        v_ptr = seq.v_ptr;

        seq.size = 0;
        seq.capacity = 0;
        seq.v_ptr = nullptr;
        return *this;
    }
    CB_Dynamic_seq(CB_Dynamic_seq&& seq) { *this = std::move(seq); }


    T& operator[](uint32_t index) {
        if (index >= size) set(index, T());
        return v_ptr[index];
    }
    T operator[](uint32_t index) const { return get(index); }


    template<typename T2> operator==(const CB_Dynamic_seq<T2>& seq) const { return false; }
    bool operator==(const CB_Dynamic_seq<T>& seq) const {
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
        const CB_Dynamic_seq& seq;
        iterator(const CB_Dynamic_seq& seq, uint32_t i=0) : seq{seq}, index{i} {}
        const T& operator*() const { ASSERT(index < seq.size); return seq.v_ptr[index]; }
        iterator& operator++() { ++index; return *this; }
        bool operator==(const iterator& o) const { return index == o.index; }
        bool operator!=(const iterator& o) const { return !(*this == o); }
        bool operator<(const iterator& o) const { return index < o.index; }
    };

    iterator begin() const { return iterator(*this); }
    iterator end() const { return iterator(*this, size); }
};

template<typename T>
CB_Type CB_Dynamic_seq<T>::type = CB_Type("[]"+T::type.toS(), sizeof(CB_Dynamic_seq<T>), CB_Dynamic_seq<T>());






/*
// Problem: Partial template of functions doesn't work

// copy
template<typename T>
CB_Dynamic_seq<T>& CB_Dynamic_seq<T>::operator=(const CB_Dynamic_seq<T>& seq) {
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
CB_Dynamic_seq<PT>& CB_Dynamic_seq<PT>::operator=(const CB_Dynamic_seq<PT>& seq) {
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






