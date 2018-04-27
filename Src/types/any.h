#pragma once

#include "type.h"
#include "pointers.h"
#include <cstring> // memcpy

/*
The type Any can be assigned a value of any type. The type information is then stored alongside the object,
and when the Any is destroyed, the actual object will also be destroyed properly.

Only classes which implement CB_Object can be used with CB_Any.
*/

struct CB_Any : CB_Object {
    static CB_Type type; // type any
    static const bool primitive = false;
    CB_Type v_type; // the type of v
    CB_Object* v_ptr = nullptr;

    CB_Any() {}
    ~CB_Any() { delete(v_ptr); v_ptr = nullptr; }

    bool has_value(const CB_Type& t) const {
        return t == v_type && v_ptr != nullptr;
    }

    std::string toS() const override {
        if (v_ptr) return v_ptr->toS();
        else return "any(void)";
    }

    CB_Object* heap_copy() const override { CB_Any* tp = new CB_Any(); *tp = *this; return tp; }

    CB_Any& operator=(const CB_Any& any) {
        v_ptr = any.v_ptr->heap_copy();
        v_type = any.v_type;
        memcpy(v_ptr, any.v_ptr, v_type.byte_size()); // @warning this might not be safe
    }
    CB_Any(const CB_Any& any) { *this = any; }

    CB_Any& operator=(CB_Any&& any) {
        this->~CB_Any();
        v_type = any.v_type;
        v_ptr = any.v_ptr;
        any.v_ptr = nullptr;
    }
    CB_Any(CB_Any&& any) { *this = std::move(any); }



    // static context only
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
    CB_Sharing_pointer<T> get_shared() {
        ASSERT(v_ptr != nullptr);
        ASSERT(T::type == v_type);
        return CB_Sharing_pointer<T>((T*)v_ptr);
    }

    template<typename T, typename Type=CB_Type*, Type=&T::type>
    CB_Any& operator=(const T& t) {
        ASSERT(T::type != CB_Any::type);
        ASSERT(T::type == t.type);
        if (T::type == v_type && v_ptr != nullptr) {
            *(T*)v_ptr = T(t);
        } else {
            allocate<T>(false);
            ASSERT(v_ptr == nullptr);
            v_ptr = new T(t);
        }
    }
    template<typename T, typename Type=CB_Type*, Type=&T::type>
    CB_Any(const T& t) { *this = t; }

    template<typename T, typename Type=CB_Type*, Type=&T::type>
    CB_Any& operator=(T&& t) {
        ASSERT(T::type != CB_Any::type);
        ASSERT(T::type == t.type);
        if (T::type == v_type && v_ptr != nullptr) {
            *(T*)v_ptr = std::move(t);
        } else {
            allocate<T>(false);
            ASSERT(v_ptr == nullptr);
            v_ptr = new T(std::move(t));
        }
    }
    template<typename T, typename Type=CB_Type const*, Type=&T::type>
    CB_Any(T&& t) { *this = std::move(t); }

    template<typename T, typename Type=CB_Type const*, Type=&T::type>
    void allocate(bool init=true) {
        this->~CB_Any();
        v_type = T::type;
        if (init) {
            v_ptr = new T();
            ASSERT(v_ptr != nullptr); // FIXME: better handling of bad alloc
        }
    }

};




