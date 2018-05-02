#pragma once

#include "assert.h"

#include <string>
#include <sstream>
#include <type_traits>

using std::nullptr_t;

/*
There are two types of pointers: owning and sharing.
An owning pointer owns an object on the heap, and will delete it when itself is deleted.
A sharing pointer points to an object owned by something else, and will never delete it.

Syntax:
op : *!T = alloc(t); // t is copied to the heap and that copy is now owned by op. // TODO: find better syntax for allocation
sp : *T = op; // sp now points to the object owned by op.
sp2 : *T = sp; // sp and sp2 now points to the same object.
op2 : *!T = op; // op2 grabs the object from op and is now the owner of the object. op is now null.
// op : *!T = sp; // illegal, sp doesn't own the object so op can't grab it

// sharing pointers are automatically casted to their base type.
t1 : T;
sp1 : *T = t1; // sp1 now points to t1
t2 : T = sp1; // t2 is now a copy of t1
sp2 : *T = sp1; // sp2 now point to the same object as sp1 points 2

*sp1 = t2; // the value that sp1 points to is now a copy of t2
sp1 = t2; // sp1 now points to t2 (implicit "adress of")

// owning pointers are automatically casted to their base type, but not for assignment
op1 : *!T = alloc(T); // has to use alloc here to allocate on the heap
t2 : T = op1; // t is now a copy of the data that op1 points to (owns)
op2 : *!T = op1; // grabs the object from op and is now the owner of the object. op is now null.
sp : *T = op1; // sp now points at the object that op1 is pointing to.
spp : **T = op1; // spp now points to op1 (pointer to a pointer to a T)
spp : **!T = op1; // spp now points to op1 (poiter to a owning pointer of T)
// spp : *!*T = op1; // not allowed, spp can never own op1 (pointer that owns a pointer to a T)

*op1 = t2; // the value that op1 points to is now a copy of t2
// op1 = t2; // not allowed, since op1 can never own t1


adress_of($T t) -> *T {
    tp : *T = t;
    return tp;
}

alloc($T : type)-> *!T {
    ptr : *!T;
    // predefined allocation of a t
    return ptr;
}

*/



/*
Owned should have deleted copy construcor and copy assignment operator.
However, that gives lots of problems with template classes such as Seq<OP> and any (assignment callbacks).


*/

template<typename T> struct Owned;
template<typename T> Owned<T> alloc(const T& t);
template<typename T> Owned<T> alloc(T&& t);
template<typename T, bool> struct deep_copy;



// owning pointer - owns the object and deallocates it when done.
template<typename T>
struct Owned {
    T* v = nullptr;

    std::string toS() const {
        std::ostringstream oss;
        oss << "*!(" << std::hex << v << ")";
        return oss.str();
    }

    // default constructor
    Owned() {}
    ~Owned() {
        if (v != nullptr) {
            v->~T();
            free(v);
        }
        v = nullptr;
    }

    T* operator->() const { ASSERT(v != nullptr); return v; }
    T& operator*() const { ASSERT(v != nullptr); return *v; }
    operator bool() const { return v != nullptr; }
    explicit operator T*() const { return v; }

    // copy - not allowed (makes a deep copy?). If passing a owned_ptr to a function, move constructor should be used. (In CB this will be done automatically)
    Owned& operator=(const Owned& ptr)
    {
        if (v == nullptr) *this = std::move(ptr.deep_copy());
        else *v = *ptr;
    }
    Owned(const Owned& ptr) { *this = ptr; }

    Owned& operator=(const nullptr_t& ptr) { ASSERT(ptr == nullptr); this->~Owned(); return *this; }
    Owned(const nullptr_t& ptr) { *this = ptr; }

    // move
    Owned& operator=(Owned&& ptr) {
        this->~Owned();
        v = ptr.v;
        ptr.v = nullptr;
        return *this;
    }
    Owned(Owned&& ptr) { *this = std::move(ptr); }

    Owned& operator=(T*&& ptr) {
        this->~Owned();
        v = ptr;
        ptr = nullptr;
        return *this;
    }
    Owned(T*&& ptr) { *this = std::move(ptr); }

    Owned deep_copy() const {
        return ::deep_copy<T, std::is_copy_constructible<T>::value>()(*this);
    }
};


template<typename T>
Owned<T> alloc(const T& t) {
    Owned<T> ptr;
    ptr.v = (T*)malloc(sizeof(T));
    ASSERT(ptr.v != nullptr);
    new (ptr.v) T(t);
    return ptr;
}

template<typename T>
Owned<T> alloc(T&& t) {
    Owned<T> ptr;
    ptr.v = (T*)malloc(sizeof(T));
    new (ptr.v) T(std::move(t));
    return ptr;
}

template<typename T> struct deep_copy<T,true> { // copy constructible
    Owned<T> operator()(const Owned<T>& ptr) {
        if (ptr == nullptr) return nullptr;
        else return alloc<T>(*ptr);
    }
};

template<typename T> struct deep_copy<T,false> { // non-copy constructible - deep copies are not allowed.
    Owned<T> operator()(const Owned<T>& ptr) { ASSERT(false); return nullptr; }
};


// sharing pointer - never deallocates the object
// shares happily with other sharing pointers
// can not be created from an object, only from other pointers
template<typename T> // T is a CB type
struct Shared {
    T* v = nullptr;

    std::string toS() const {
        std::ostringstream oss;
        oss << "*(" << std::hex << v << ")";
        return oss.str();
    }

    // default constructor
    Shared() {}
    ~Shared() { v = nullptr; }

    T* operator->() const { ASSERT(v != nullptr); return v; }
    T& operator*() const { ASSERT(v != nullptr); return *v; }
    operator bool() const { return v != nullptr; }
    explicit operator T*() const { return v; }

    // copy
    Shared& operator=(const Shared& ptr) { v = ptr.v; return *this; }
    Shared(const Shared& ptr) { *this = ptr; }

    Shared& operator=(const Owned<T>& ptr) { v = ptr.v; return *this; }
    Shared(const Owned<T>& ptr) { *this = ptr; }

    Shared& operator=(T* ptr) { v = ptr; return *this; }
    Shared(T* ptr) { *this = ptr; }

    Shared& operator=(const nullptr_t& ptr) { ASSERT(ptr == nullptr); v = ptr; return *this; }
    Shared(const nullptr_t& ptr) { *this = ptr; }
};

// comparison betweens different kinds of pointers
template<typename T> bool operator==(const Shared<T>& lhs, const Shared<T>& rhs) { return lhs.v == rhs.v; }
template<typename T> bool operator==(const Owned<T>& lhs, const Owned<T>& rhs) { return lhs.v == rhs.v; }
template<typename T> bool operator==(const Shared<T>& lhs, const Owned<T>& rhs) { return lhs.v == rhs.v; }
template<typename T> bool operator==(const Owned<T>& lhs, const Shared<T>& rhs) { return lhs.v == rhs.v; }
template<typename T> bool operator==(const Shared<T>& lhs, const T* rhs) { return lhs.v == rhs; }
template<typename T> bool operator==(const T* lhs, const Shared<T>& rhs) { return lhs == rhs.v; }
template<typename T> bool operator==(const Owned<T>& lhs, const T* rhs) { return lhs.v == rhs; }
template<typename T> bool operator==(const T* lhs, const Owned<T>& rhs) { return lhs == rhs.v; }
template<typename T> bool operator==(const Shared<T>& lhs, const nullptr_t rhs) { return lhs.v == rhs; }
template<typename T> bool operator==(const nullptr_t lhs, const Shared<T>& rhs) { return lhs == rhs.v; }
template<typename T> bool operator==(const Owned<T>& lhs, const nullptr_t rhs) { return lhs.v == rhs; }
template<typename T> bool operator==(const nullptr_t lhs, const Owned<T>& rhs) { return lhs == rhs.v; }

template<typename T> bool operator!=(const Shared<T>& lhs, const Shared<T>& rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const Owned<T>& lhs, const Owned<T>& rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const Shared<T>& lhs, const Owned<T>& rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const Owned<T>& lhs, const Shared<T>& rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const Shared<T>& lhs, const T* rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const T* lhs, const Shared<T>& rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const Owned<T>& lhs, const T* rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const T* lhs, const Owned<T>& rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const Shared<T>& lhs, const nullptr_t rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const nullptr_t lhs, const Shared<T>& rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const Owned<T>& lhs, const nullptr_t rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const nullptr_t lhs, const Owned<T>& rhs) { return !(lhs == rhs); }

template<typename T, typename T2>
Shared<T> dynamic_pointer_cast(const Owned<T2>& ptr) {
    return Shared<T>(dynamic_cast<T*>(ptr.v));
}
template<typename T, typename T2>
Shared<T> dynamic_pointer_cast(const Shared<T2>& ptr) {
    return Shared<T>(dynamic_cast<T*>(ptr.v));
}
template<typename T, typename T2>
Shared<T> static_pointer_cast(const Owned<T2>& ptr) {
    return Shared<T>(static_cast<T*>(ptr.v));
}
template<typename T, typename T2>
Shared<T> static_pointer_cast(const Shared<T2>& ptr) {
    return Shared<T>(static_cast<T*>(ptr.v));
}

template<typename T, typename T2>
Owned<T> owning_pointer_cast(Owned<T2>&& ptr) {
    auto p = Owned<T>(dynamic_cast<T*>(ptr.v));
    if (p.v != nullptr) ptr.v = nullptr;
    return p;
}


