#pragma once

#include "type.h"

/*
The type Any can be assigned a value of any type. The type information is then stored alongside the object,
and when the Any is destroyed, the actual object will also be destroyed properly.

In c++, this is done through templated callbacks, so the struct is a bit bigger than it should be in Cube.
*/

struct CB_Any {
    static CB_Type type; // type any
    CB_Type v_type; // the type of v
    void* v_ptr = nullptr;

    std::string toS() const { return toS_callback(*this); }

    CB_Any() { set_default_callbacks(); }
    ~CB_Any() {
        destructor_callback(*this);
        set_default_callbacks();
    }

    template<typename T>
    T& value() {
        ASSERT(v_ptr != nullptr);
        ASSERT(T::type == v_type);
        return *(T*)v_ptr;
    }

    template<typename T>
    CB_Any& operator=(const T& t) {
        allocate<T>(false);
        new (v_ptr) T(t);
    }
    template<typename T>
    CB_Any(const T& t) { *this = t; }

    template<typename T>
    CB_Any& operator=(T&& t) {
        allocate<T>(false);
        new (v_ptr) T(std::move(t));
    }
    template<typename T>
    CB_Any(T&& t) { *this = std::move(t); }

    template<typename T>
    void allocate(bool init=true) {
        this->~CB_Any();
        v_type = T::type;
        v_ptr = malloc(sizeof(T));
        if (init) new ((T*)v_ptr) T();
        set_callbacks<T>();
    }

private:
    void (*destructor_callback)(CB_Any&); // only compile time
    std::string (*toS_callback)(const CB_Any&); // only compile time

    template<typename T> void set_callbacks();
    void set_default_callbacks();
};
CB_Type CB_Any::type = CB_Type("any");


template<typename T>
std::string toS_any_callback(const CB_Any& any) {
    ASSERT(T::type == any.v_type);
    return ((T*)any.v_ptr)->toS();
}
template<typename T>
void destroy_any_callback(CB_Any& any)
{
    ((T*)any.v_ptr)->~T();
    free(any.v_ptr);
    any.v_ptr = nullptr;
}

static std::string toS_null_callback(const CB_Any& any) { ASSERT(any.v_ptr == nullptr); return "any(null)"; }
static void destroy_null_callback(CB_Any& any) { ASSERT(any.v_ptr == nullptr); }


template<typename T>
void CB_Any::set_callbacks() {
    destructor_callback = destroy_any_callback<T>;
    toS_callback = toS_any_callback<T>;
}

void CB_Any::set_default_callbacks() {
    destructor_callback = destroy_null_callback;
    toS_callback = toS_null_callback;
}