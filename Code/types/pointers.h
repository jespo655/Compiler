#pragma once

#include "type.h"

#include <string>
#include <sstream>

/*
There are two types of pointers: owning and sharing.
An owning pointer owns an object on the heap, and will delete it when itself is deleted.
A sharing pointer points to an object owned by something else, and will never delete it.

Syntax:
op : T*! = alloc(t); // t is copied to the heap and that copy is now owned by op. // TODO: find better syntax for allocation
sp : T* = op; // sp now points to the object owned by op.
sp2 : T* = sp; // sp and sp2 now points to the same object.
op2 : T*! = op; // op2 grabs the object from op and is now the owner of the object. op is now null.
// op : T*! = sp; // illegal, sp doesn't own the object so op can't grab it
*/



/*
CB_Owning_pointer should have deleted copy construcor and copy assignment operator.
However, that gives lots of problems with template classes such as seq<OP> and any (assignment callbacks).


*/

template<typename T> struct CB_Owning_pointer;
template<typename T> CB_Owning_pointer<T> alloc(const T& t);
template<typename T> CB_Owning_pointer<T> alloc(T&& t);
template<typename T, bool> struct deep_copy;



// template<typename T, bool> struct assign_any_callback {};

// template<typename T>
// struct assign_any_callback<T,true>{
//     static void f(CB_Any& obj, const CB_Any& any) {
//         ASSERT(T::type == any.v_type);
//         obj = any.value<T>();
//     }
// };
// template<typename T>
// struct assign_any_callback<T,false>{
//     static void f(CB_Any& obj, const CB_Any& any) {
//         ASSERT(false, "Non copy constructible objects can't be copied through any!");
//     }
// };

    // assign_callback = assign_any_callback<T, std::is_copy_constructible<T>::value>::f;






// owning pointer - owns the object and deallocates it when done.
template<typename T> // T is a CB type
struct CB_Owning_pointer {
    static CB_Type type;
    T* v = nullptr;

    std::string toS() const {
        std::ostringstream oss;
        oss << "pointer!(" << std::hex << v << ")";
        return oss.str();
    }

    // default constructor
    CB_Owning_pointer() {}
    ~CB_Owning_pointer() {
        if (v != nullptr) {
            v->~T();
            free(v);
        }
        v = nullptr;
    }

    T* operator->() const { ASSERT(v != nullptr); return v; }
    T& operator*() const { ASSERT(v != nullptr); return *v; }

    // copy - not allowed (makes a deep copy?). If passing a owned_ptr to a function, move constructor should be used. (In CB this will be done automatically)
    CB_Owning_pointer& operator=(const CB_Owning_pointer& ptr)
    {
        if (v == nullptr) *this = std::move(ptr.deep_copy());
        else *v = *ptr;
    }
    CB_Owning_pointer(const CB_Owning_pointer& ptr) { *this = ptr; }

    CB_Owning_pointer& operator=(const nullptr_t& ptr) { ASSERT(ptr == nullptr); this->~CB_Owning_pointer(); return *this; }
    CB_Owning_pointer(const nullptr_t& ptr) { *this = ptr; }

    // move
    CB_Owning_pointer& operator=(CB_Owning_pointer&& ptr) {
        this->~CB_Owning_pointer();
        v = ptr.v;
        ptr.v = nullptr;
        return *this;
    }
    CB_Owning_pointer(CB_Owning_pointer&& ptr) { *this = std::move(ptr); }

    CB_Owning_pointer& operator=(T*&& ptr) {
        this->~CB_Owning_pointer();
        v = ptr;
        ptr = nullptr;
        return *this;
    }
    CB_Owning_pointer(T*&& ptr) { *this = std::move(ptr); }

    CB_Owning_pointer deep_copy() const {
        return ::deep_copy<T, std::is_copy_constructible<T>::value>()(*this);
        // return alloc<T>(*v);
    }
};
template<typename T>
// CB_Type CB_Owning_pointer<T>::type = CB_Type("*!"+T::type.toS());
CB_Type CB_Owning_pointer<T>::type = CB_Type("*!"+T::type.toS(), CB_Owning_pointer<T>());


template<typename T>
CB_Owning_pointer<T> alloc(const T& t) {
    CB_Owning_pointer<T> ptr;
    ptr.v = (T*)malloc(sizeof(T));
    ASSERT(ptr.v != nullptr);
    new (ptr.v) T(t);
    return ptr;
}

template<typename T>
CB_Owning_pointer<T> alloc(T&& t) {
    CB_Owning_pointer<T> ptr;
    ptr.v = (T*)malloc(sizeof(T));
    new (ptr.v) T(std::move(t));
    return ptr;
}

template<typename T> struct deep_copy<T,true> { // copy constructible
    CB_Owning_pointer<T> operator()(const CB_Owning_pointer<T>& ptr) {
        if (ptr == nullptr) return nullptr;
        else return alloc<T>(*ptr);
    }
};
template<typename T> struct deep_copy<T,false> { // non-copy constructible - deep copies are not allowed.
    CB_Owning_pointer<T> operator()(const CB_Owning_pointer<T>& ptr) { ASSERT(false); return nullptr; }
};

// template<typename T, bool> struct assign_any_callback {};

// template<typename T>
// struct assign_any_callback<T,true>{
//     static void f(CB_Any& obj, const CB_Any& any) {
//         ASSERT(T::type == any.v_type);
//         obj = any.value<T>();
//     }
// };
// template<typename T>
// struct assign_any_callback<T,false>{
//     static void f(CB_Any& obj, const CB_Any& any) {
//         ASSERT(false, "Non copy constructible objects can't be copied through any!");
//     }
// };

    // assign_callback = assign_any_callback<T, std::is_copy_constructible<T>::value>::f;









// sharing pointer - never deallocates the object
// shares happily with other sharing pointers
// can not be created from an object, only from other pointers
template<typename T> // T is a CB type
struct CB_Sharing_pointer {
    static CB_Type type;
    T* v = nullptr;

    std::string toS() const {
        std::ostringstream oss;
        oss << "pointer(" << std::hex << v << ")";
        return oss.str();
    }

    // default constructor
    CB_Sharing_pointer() {}
    ~CB_Sharing_pointer() { v = nullptr; }

    T* operator->() const { ASSERT(v != nullptr); return v; }
    T& operator*() const { ASSERT(v != nullptr); return *v; }

    // copy
    CB_Sharing_pointer& operator=(const CB_Sharing_pointer& ptr) { v = ptr.v; return *this; }
    CB_Sharing_pointer(const CB_Sharing_pointer& ptr) { *this = ptr; }

    CB_Sharing_pointer& operator=(const CB_Owning_pointer<T>& ptr) { v = ptr.v; return *this; }
    CB_Sharing_pointer(const CB_Owning_pointer<T>& ptr) { *this = ptr; }

    CB_Sharing_pointer& operator=(T* ptr) { v = ptr; return *this; }
    CB_Sharing_pointer(T* ptr) { *this = ptr; }

    CB_Sharing_pointer& operator=(const nullptr_t& ptr) { ASSERT(ptr == nullptr); v = ptr; return *this; }
    CB_Sharing_pointer(const nullptr_t& ptr) { *this = ptr; }
};
template<typename T>
CB_Type CB_Sharing_pointer<T>::type = CB_Type("*"+T::type.toS(), CB_Sharing_pointer<T>());

// comparison betweens different kinds of pointers
template<typename T> bool operator==(const CB_Sharing_pointer<T>& lhs, const CB_Sharing_pointer<T>& rhs) { return lhs.v == rhs.v; }
template<typename T> bool operator==(const CB_Owning_pointer<T>& lhs, const CB_Owning_pointer<T>& rhs) { return lhs.v == rhs.v; }
template<typename T> bool operator==(const CB_Sharing_pointer<T>& lhs, const CB_Owning_pointer<T>& rhs) { return lhs.v == rhs.v; }
template<typename T> bool operator==(const CB_Owning_pointer<T>& lhs, const CB_Sharing_pointer<T>& rhs) { return lhs.v == rhs.v; }
template<typename T> bool operator==(const CB_Sharing_pointer<T>& lhs, const T* rhs) { return lhs.v == rhs; }
template<typename T> bool operator==(const T* lhs, const CB_Sharing_pointer<T>& rhs) { return lhs == rhs.v; }
template<typename T> bool operator==(const CB_Owning_pointer<T>& lhs, const T* rhs) { return lhs.v == rhs; }
template<typename T> bool operator==(const T* lhs, const CB_Owning_pointer<T>& rhs) { return lhs == rhs.v; }
template<typename T> bool operator==(const CB_Sharing_pointer<T>& lhs, const nullptr_t rhs) { return lhs.v == rhs; }
template<typename T> bool operator==(const nullptr_t lhs, const CB_Sharing_pointer<T>& rhs) { return lhs == rhs.v; }
template<typename T> bool operator==(const CB_Owning_pointer<T>& lhs, const nullptr_t rhs) { return lhs.v == rhs; }
template<typename T> bool operator==(const nullptr_t lhs, const CB_Owning_pointer<T>& rhs) { return lhs == rhs.v; }

template<typename T> bool operator!=(const CB_Sharing_pointer<T>& lhs, const CB_Sharing_pointer<T>& rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const CB_Owning_pointer<T>& lhs, const CB_Owning_pointer<T>& rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const CB_Sharing_pointer<T>& lhs, const CB_Owning_pointer<T>& rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const CB_Owning_pointer<T>& lhs, const CB_Sharing_pointer<T>& rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const CB_Sharing_pointer<T>& lhs, const T* rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const T* lhs, const CB_Sharing_pointer<T>& rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const CB_Owning_pointer<T>& lhs, const T* rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const T* lhs, const CB_Owning_pointer<T>& rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const CB_Sharing_pointer<T>& lhs, const nullptr_t rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const nullptr_t lhs, const CB_Sharing_pointer<T>& rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const CB_Owning_pointer<T>& lhs, const nullptr_t rhs) { return !(lhs == rhs); }
template<typename T> bool operator!=(const nullptr_t lhs, const CB_Owning_pointer<T>& rhs) { return !(lhs == rhs); }

template<typename T, typename T2>
CB_Sharing_pointer<T> dynamic_pointer_cast(const CB_Owning_pointer<T2>& ptr) {
    return CB_Sharing_pointer<T>(dynamic_cast<T*>(ptr.v));
}
template<typename T, typename T2>
CB_Sharing_pointer<T> dynamic_pointer_cast(const CB_Sharing_pointer<T2>& ptr) {
    return CB_Sharing_pointer<T>(dynamic_cast<T*>(ptr.v));
}

