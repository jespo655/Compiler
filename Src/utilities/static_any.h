#pragma once

#include "../types/cb_type.h"
#include "pointers.h"

/*
The type Any can be assigned a value of any type. The type information is then stored alongside the object,
and when the Any is destroyed, the actual object will also be destroyed properly.

In c++, this is done through templated callbacks, so the struct is a bit bigger than it should be in Cube.
*/

namespace Cube {

struct Static_any {
    static CB_Type type; // type any
    static const bool primitive = false;
    CB_Type v_type; // the type of v
    void* v_ptr = nullptr;

    std::string toS() const { return toS_callback(*this); }

    Static_any() { /*std::cout << "cstr ";*/ set_default_callbacks(); }
    ~Static_any() {
        destructor_callback(*this);
        /*std::cout << "dstr ";*/ set_default_callbacks();
    }

    template<typename T, typename Type=CB_Type const*, Type=&T::type>
    const T& value() const {
        ASSERT(v_ptr != nullptr);
        ASSERT(T::type == v_type);
        return *(T const *)v_ptr;
    }

    template<typename T, typename Type=CB_Type*, Type=&T::type>
    T& value() {
        ASSERT(v_ptr != nullptr);
        ASSERT(T::type == v_type);
        return *(T*)v_ptr;
    }

    template<typename T, typename Type=CB_Type const*, Type=&T::type>
    Shared<T> get_shared() {
        ASSERT(v_ptr != nullptr);
        ASSERT(T::type == v_type);
        return Shared<T>((T*)v_ptr);
    }

    bool has_value(const CB_Type& t) const {
        return t == v_type && v_ptr != nullptr;
    }

    Static_any& operator=(const Static_any& any);
    Static_any(const Static_any& any) { /*std::cout << "copy any cstr ";*/ set_default_callbacks(); *this = any; }

    template<typename T, typename Type=CB_Type*, Type=&T::type>
    Static_any& operator=(const T& t) {
        // std::cout << "any copy op = " << T::type.toS() << std::endl;
        ASSERT(T::type != Static_any::type);
        ASSERT(T::type == t.type);
        if (T::type == v_type && v_ptr != nullptr) {
            *(T*)v_ptr = T(t);
        } else {
            allocate<T>(false);
            new (v_ptr) T(t);
        }
    }

    template<typename T, typename Type=CB_Type*, Type=&T::type>
    Static_any(const T& t) { /*std::cout << "copy " << t.type.toS() << " cstr ";*/ set_default_callbacks(); *this = t; }

    // Static_any& operator=(Static_any&& any) = delete;
    Static_any& operator=(Static_any&& any);
    Static_any(Static_any&& any) { /*std::cout << "move cstr ";*/ set_default_callbacks(); *this = std::move(any); }

    template<typename T, typename Type=CB_Type*, Type=&T::type>
    Static_any& operator=(T&& t) {
        // std::cout << "any move op = " << T::type.toS() << std::endl;
        ASSERT(T::type != Static_any::type);
        ASSERT(T::type == t.type);
        if (T::type == v_type && v_ptr != nullptr) {
            *(T*)v_ptr = std::move(t);
        } else {
            allocate<T>(false);
            new (v_ptr) T(std::move(t));
        }
    }
    template<typename T, typename Type=CB_Type const*, Type=&T::type>
    Static_any(T&& t) { /*std::cout << "move " << T::type.toS() << " cstr ";*/ set_default_callbacks(); *this = std::move(t); }

    template<typename T, typename Type=CB_Type const*, Type=&T::type>
    void allocate(bool init=true) {
        this->~Static_any();
        v_type = T::type;
        v_ptr = malloc(sizeof(T));
        ASSERT(v_ptr != nullptr); // FIXME: better handling of bad alloc
        if (init) new ((T*)v_ptr) T();
        /*std::cout << "alloc " << T::type.toS() << " ";*/ set_callbacks<T>();
    }

private:
    void (*destructor_callback)(Static_any&); // only compile time
    std::string (*toS_callback)(const Static_any&); // only compile time
    void (*assign_callback)(Static_any&, const Static_any&); // only compile time

    template<typename T> void set_callbacks();
    void set_default_callbacks();
};


template<typename T>
std::string toS_any_callback(const Static_any& any) {
    ASSERT(T::type == any.v_type);
    ASSERT(any.v_ptr != nullptr);
    return ((T*)any.v_ptr)->toS();
}
template<typename T>
void destroy_any_callback(Static_any& any)
{
    ASSERT(T::type == any.v_type);
    ASSERT(any.v_ptr != nullptr, "Trying to delete null of type "+T::type.toS());
    ((T*)any.v_ptr)->~T();
    free(any.v_ptr);
    any.v_ptr = nullptr;
}
template<typename T>
void assign_any_callback(Static_any& obj, const Static_any& any)
{
    ASSERT(T::type == any.v_type);
    // std::cout << "assign any callback, current value=" << obj.toS() << ", new value=" << any.toS() << std::endl;
    obj = any.value<T>();
}

template<typename T>
void Static_any::set_callbacks() {
    // std::cout << "setting callbacks for type " << T::type.toS() << std::endl;
    destructor_callback = destroy_any_callback<T>;
    toS_callback = toS_any_callback<T>;
    assign_callback = assign_any_callback<T>;
    // assign_callback = assign_any_callback<T, std::is_copy_constructible<T>::value>::f;
}

}
