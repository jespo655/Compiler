#pragma once

#include "../types/cb_type.h"
#include "pointers.h"
#include "debug.h"

#include <functional>

/*
The type Any can be assigned a value of any type. The type information is then stored alongside the object,
and when the Any is destroyed, the actual object will also be destroyed properly.

In c++, this is done through templated callbacks, so the struct is a bit bigger than it should be in Cube.
*/

struct Static_any : public Serializable {

private:
    // Compile-time uids for generic c++ classes, taken from https://stackoverflow.com/questions/36983179/constexpr-id-for-class-supporting-equality-comparison#36984124
    template<typename T>
    static void id_gen(){}
    using type_id_t = void(*)(void);
    template<typename T>
    constexpr static type_id_t type_id = &id_gen<T>;

public:
    type_id_t type = type_id<void>;
    void* v_ptr = nullptr;

    std::string toS() const override {
        ASSERT(callbacks);
        return callbacks->toS(*this);
    }

    Static_any() {
        set_callbacks<void>();
        printf("constructor end\n");
    }
    ~Static_any() {
        printf("destructor start\n");
        if (callbacks) {
            // destroy_callback(*this);
            delete callbacks;
            callbacks = nullptr;
        } else {
            if (v_ptr) free(v_ptr);
            v_ptr = nullptr;
        }
        printf("destructor end\n");
    }

    template<typename T>
    const T& value() const {
        ASSERT(has_value<T>());
        return *(T const *)v_ptr;
    }

    template<typename T>
    T& value() {
        ASSERT(has_value<T>());
        return *(T*)v_ptr;
    }

    template<typename T>
    Shared<T> get_shared() {
        ASSERT(has_value<T>());
        return Shared<T>((T*)v_ptr);
    }

    template<typename T>
    bool has_type() const {
        return type == type_id<T>;
    }

    template<typename T>
    bool has_value() const {
        return has_type<T>() && v_ptr != nullptr;
    }

    template<typename T>
    bool has_value(const T& t) const {
        return has_value<T>() && (*(T*)v_ptr) == t;
    }

    Static_any& operator=(const Static_any& any) {
        any.callbacks->copy(*this, any);
    }
    Static_any(const Static_any& any) { set_callbacks<void>(); *this = any; }

    template<typename T>
    Static_any& operator=(const T& t) {
        if (has_value<T>()) {
            *(T*)v_ptr = t;
        } else {
            allocate<T>(false);
            new (v_ptr) T(t);
        }
    }
    template<typename T>
    Static_any(const T& t) { set_callbacks<void>(); *this = t; }

    Static_any& operator=(Static_any&& any) {
        this->~Static_any();
        v_ptr = any.v_ptr;
        callbacks = any.callbacks;
        any.v_ptr = nullptr;
        any.set_callbacks<void>();
    }
    Static_any(Static_any&& any) { *this = std::move(any); }

    template<typename T, class = typename std::enable_if<!std::is_lvalue_reference<T>::value>::type>
    Static_any& operator=(T&& t) {
        if (has_value<T>()) {
            *(T*)v_ptr = std::move(t);
        } else {
            allocate<T>(false);
            new (v_ptr) T(std::move(t));
        }
    }
    template<typename T, class = typename std::enable_if<!std::is_lvalue_reference<T>::value>::type>
    Static_any(T&& t) { set_callbacks<void>(); *this = std::move(t); }

    template<typename T>
    void allocate(bool init=true) {
        this->~Static_any();
        v_ptr = malloc(sizeof(T));
        ASSERT(v_ptr != nullptr); // FIXME: better handling of bad alloc
        if (init) new ((T*)v_ptr) T();
        set_callbacks<T>();
    }

    template<typename T> bool operator==(const T& o) const { return has_value<T>() && value<T>() == o; }
    bool operator==(const Static_any& o) const { return callbacks->compare(*this, o); }
    template<typename T> bool operator!=(const T& o) const { return !(*this == o); }

private:

    struct Any_callbacks {
        virtual std::string toS(const Static_any& any) = 0;
        virtual void copy(Static_any& obj, const Static_any& any) = 0;
        virtual bool compare(const Static_any& a1, const Static_any& a2) = 0;
    };

    template<typename T, bool = std::is_base_of<Serializable, T>::value>
    struct T_callbacks : Any_callbacks {

        std::string toS(const Static_any& any) {
            return "any(non-serializable type)";
        }

        void copy(Static_any& obj, const Static_any& any) {
            obj = any.value<T>();
        }

        bool compare(const Static_any& a1, const Static_any& a2) {
            return a1.has_value<T>() && a2.has_value<T>() && *(T*)a1.v_ptr == *(T*)a2.v_ptr;
        }
    };

    Any_callbacks* callbacks;
    void (*destroy_callback)(Static_any& any); // can't call virtual functions in destructor so we need this special case

    template<typename T> void set_callbacks();

};





template<typename T>
struct Static_any::T_callbacks<T, true> : T_callbacks<T, false> {
    std::string toS(const Static_any& any) override {
        return ((Serializable*)any.v_ptr)->toS();
    }
};

template<>
struct Static_any::T_callbacks<void, false> : Any_callbacks {

    std::string toS(const Static_any& any) override {
        ASSERT(any.v_ptr == nullptr);
        return "any(void)";
    }

    void copy(Static_any& obj, const Static_any& any) override {
        ASSERT(any.v_ptr == nullptr);
        obj.~Static_any();
    }

    bool compare(const Static_any& a1, const Static_any& a2) override {
        return a1.has_type<void>() && a2.has_type<void>();
    }
};

template<typename T>
void destroy_any(Static_any& any) {
    if (any.v_ptr != nullptr) {
        ((T*)any.v_ptr)->~T();
        free(any.v_ptr);
        any.v_ptr = nullptr;
    }
}

static void destroy_void(Static_any& any) {
    printf("destroy void\n");
    ASSERT(any.v_ptr == nullptr);
}

template<typename T> void Static_any::set_callbacks() {
    if (!has_type<T>()) {
        if (callbacks != nullptr) delete callbacks;
        callbacks = new T_callbacks<T, std::is_base_of<Serializable, T>::value>;
        destroy_callback = destroy_void;
    }
};
