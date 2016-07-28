#pragma once

// #include "seq.h"
// #include "numbers.h"
// #include "type.h"


// #include "../utilities/unique_id.h"
#include "../utilities/assert.h"

#include <string>
#include <sstream>
#include <tuple>
#include <cstring>



// problem: cannot be declared static in a header file - that
// will make separate instances of "id" in different files
int get_unique_id() {
    static int id=0; // -1 is uninitialized
    ASSERT(id >= 0); // if id is negative then the int has looped around. More than INT_MAX unique identifiers should never be needed.
    return id++;
}





/*
This is a list of built in types
Each built in type is represented by a c++ struct
An instance of a type can have a value of that type

Things that is supplied here:

* The field 'static CB_Type type', which is
    an unique type identifier. It does not take any space,
    and is always constexpr in compiled code.
    Supplied for each type.

* Implicit cast operators (if applicable)

* std::string toS() function
* void destroy() function, might be empty



Built in functions that refer to types:
* sizeof(Type t)

*/





struct CB_Type
{
    static CB_Type type; // self reference / CB_Type
    int uid = get_unique_id();

    void destroy() {}
    std::string toS() const {
        return "type("+std::to_string(uid)+")";
    }

    bool operator==(CB_Type& o) const { return uid == o.uid; }
    bool operator!=(CB_Type& o) const { return !(*this==o); }
};
CB_Type CB_Type::type = CB_Type();


template<typename T> // T is a CB type
struct CB_Dynamic_seq {
    static CB_Type type;
    constexpr static CB_Type& member_type = T::type;
    constexpr static bool dynamic = true;
    int size = 0;
    int capacity = 16; // arbitrary power of 2
    T* v_ptr = nullptr;
    int id = get_unique_id();

    void destroy() {
        for (int i = 0; i < size; ++i) {
            v_ptr[i].~T();
        }
        free(v_ptr);
        v_ptr = nullptr;
        size = 0;
        capacity = 0;
    }

    std::string toS() const {
        std::ostringstream oss;
        oss << "[" << type.toS() << ", dynamic: ";
        for (int i = 0; i < size; ++i) {
            if (i != 0) oss << ", ";
            oss << v_ptr[i].toS();
        }
        oss << "]";
        return oss.str();
    }

    CB_Dynamic_seq() {
        size = 0;
        capacity = 16; // arbitrary power of 2
        id = get_unique_id();
        v_ptr = (T*)malloc(capacity*sizeof(T));
        // size is always 0 at init, so no need to init members
    }

    ~CB_Dynamic_seq() { destroy(); }

    // copy
    CB_Dynamic_seq& operator=(const CB_Dynamic_seq& seq) {
        for (int i = 0; i < size; ++i) {
            v_ptr[i].~T();
        }
        resize(seq.size, false);
        for (int i = 0; i < size; ++i) {
            new (&v_ptr[i]) T(seq.v_ptr[i]);
        }
    }
    CB_Dynamic_seq(const CB_Dynamic_seq& seq) { *this = seq; }

    // move
    CB_Dynamic_seq& operator=(CB_Dynamic_seq&& seq) {
        destroy();
        size = seq.size;
        capacity = seq.capacity;
        v_ptr = seq.v_ptr;

        seq.size = 0;
        seq.capacity = 0;
        seq.v_ptr = nullptr;
    }
    CB_Dynamic_seq(CB_Dynamic_seq&& seq) { *this = std::move(seq); }


    T operator[](int i) const { return get(i); }

    T get(int i) const {
        if (i >= 0 && i < size)
            return v_ptr[i];
        return T();
    }

    void set(int i, const T& t) {
        if (i >= 0 && i < size)
            v_ptr[i] = t;
        else if (i >= size) {
            while (i >= capacity) {
                reallocate(2*capacity);
            }
            for (int it = size; it < i; ++it) {
                v_ptr[it] = T(); // fill with default values
            }
            v_ptr[i] = t;
            size = i+1;
        } else {
            // i is negative -> can't set
        }
    }

    void add(const T& t) {
        set(size, t);
    }

    void resize(int new_size, bool init=true, T default_value=T()) {
        if (new_size > size) {
            if (new_size > capacity) {
                reallocate(new_size);
            }
            if (init) {
                for (int it = size; it < new_size; ++it) {
                    new (&v_ptr[it]) T(default_value); // fill with default values
                }
            }
        }
        size = new_size;
    }

    void reallocate(int capacity, bool move=true) {
        this->capacity = capacity;
        T* new_ptr = (T*)malloc(capacity*sizeof(T));
        ASSERT(new_ptr != nullptr);
        if (capacity < size) {
            for (int i = capacity; i < size; ++i) v_ptr[i].~T();
            size = capacity;
        }
        if (move) memcpy(new_ptr, v_ptr, size*sizeof(T));
        free(v_ptr);
        v_ptr = new_ptr;
    }
};

template<typename T>
CB_Type CB_Dynamic_seq<T>::type = CB_Type();


template<typename T, int c_size> // T is a CB type
struct CB_Static_seq {
    static CB_Type type;
    constexpr static CB_Type& member_type = T::type;
    constexpr static bool dynamic = false;
    constexpr static int size = c_size;
    constexpr static int capacity = c_size;
    T v[c_size];

    void destroy() {
        for (int i = 0; i < size; ++i) v[i].~T();
    }

    std::string toS() const {
        std::ostringstream oss;
        oss << "[" << type.toS() << ", size=" << size << ": ";
        for (int i = 0; i < size; ++i) {
            if (i != 0) oss << ", ";
            oss << v[i].toS();
        }
        oss << "]";
        return oss.str();
    }

    CB_Static_seq(bool init=true) {
        if (init) {
            for (int i = 0; i < size; ++i) {
                new (&v[i]) T();
            }
        }
    }
    ~CB_Static_seq() { destroy(); }

    T operator[](int i) const { return get(i); }

    T get(int i) const {
        if (i >= 0 && i < c_size)
            return v[i];
        return T();
    }

    bool set(int i, T t) {
        if (i >= 0 && i < c_size) {
            v[i] = t;
            return true;
        }
        return false;
    }

    operator CB_Dynamic_seq<T>() const
    {
        CB_Dynamic_seq<T> seq;
        seq.reallocate(size);
        seq.size = size;
        memcpy(seq.v_ptr, v, size*sizeof(T));
        return seq;
    }
};

template<typename T, int size>
CB_Type CB_Static_seq<T, size>::type = CB_Type();


// string: just like a dynamic array of characters
// actual allocated size is size+1 - the last character is always '\0'
// operating on individual chars is not allowed.
struct CB_String {
    static CB_Type type;
    int size = 0; // size in bytes, not in UTF-8 characters
    int capacity = 0;
    char* v_ptr = nullptr;

    void destroy() {
        free(v_ptr); v_ptr = nullptr;
    }

    std::string toS() const {
        ASSERT(v_ptr != nullptr);
        return std::string(v_ptr);
    }

    // default constructor
    CB_String() {
        v_ptr = (char*)malloc(capacity+1);
        memset(v_ptr, 0, capacity+1);
    }
    CB_String(const std::string& str) {
        size = str.length();
        capacity = size;
        v_ptr = (char*)malloc(size+1);
        memcpy(v_ptr, str.c_str(), size+1);
    }
    CB_String(const char* cstr) {
        size = strlen(cstr);
        capacity = size;
        v_ptr = (char*)malloc(size+1);
        memcpy(v_ptr, cstr, size+1);
    }
    ~CB_String() { destroy(); }

    // copy
    CB_String& operator=(const CB_String& str) {
        if (capacity < str.size)
            reallocate(str.size, false);
        ASSERT(capacity >= str.size);
        memcpy(v_ptr, str.v_ptr, size+1);
        size = str.size;
    }

    CB_String(const CB_String& str) {
        *this = str;
    }

    // move
    CB_String& operator=(CB_String&& str) {
        free(v_ptr);
        size = str.size;
        v_ptr = str.v_ptr;
        str.v_ptr = nullptr;
    }

    CB_String(CB_String&& str) {
        *this = std::move(str);
    }

    void reallocate(int capacity, bool move=true) {
        this->capacity = capacity;
        char* new_ptr = (char*)malloc(capacity);
        ASSERT(new_ptr != nullptr);
        if (capacity < size) size = capacity;
        if (move) memcpy(new_ptr, v_ptr, size);
        free(v_ptr);
        v_ptr = new_ptr;
    }
};

CB_Type CB_String::type = CB_Type();


struct CB_bool {
    static CB_Type type;
    bool v = false;
    void destroy() {}
    CB_bool() {}
    CB_bool(bool b) { this->v = b; }
    std::string toS() const { return v? "true" : "false"; }
};

CB_Type CB_bool::type = CB_Type();


#define CB_NUMBER_TYPE(CB_t, CPP_t)                         \
struct CB_t {                                               \
    static CB_Type type;                                    \
    CPP_t v = 0;                                            \
    void destroy() {}                                       \
    CB_t() {}                                               \
    CB_t(const CPP_t& v) { this->v = v; }                   \
    std::string toS() const { return std::to_string(v); }   \
};                                                          \
CB_Type CB_t::type = CB_Type();

CB_NUMBER_TYPE(CB_i8, int8_t);
CB_NUMBER_TYPE(CB_i16, int16_t);
CB_NUMBER_TYPE(CB_i32, int32_t);
CB_NUMBER_TYPE(CB_i64, int64_t);

CB_NUMBER_TYPE(CB_u8, uint8_t);
CB_NUMBER_TYPE(CB_u16, uint16_t);
CB_NUMBER_TYPE(CB_u32, uint32_t);
CB_NUMBER_TYPE(CB_u64, uint64_t);

CB_NUMBER_TYPE(CB_f32, float);
CB_NUMBER_TYPE(CB_f64, double);

// generic int - stored as i64 but can be implicitly casted to any integer type
struct CB_int {
    static CB_Type type;
    int64_t v;
    operator CB_i8() { CB_i8 i; i.v = v; return i; }
    operator CB_i16() { CB_i16 i; i.v = v; return i; }
    operator CB_i32() { CB_i32 i; i.v = v; return i; }
    operator CB_i64() { CB_i64 i; i.v = v; return i; }

    operator CB_u8() { CB_u8 i; i.v = v; return i; }
    operator CB_u16() { CB_u16 i; i.v = v; return i; }
    operator CB_u32() { CB_u32 i; i.v = v; return i; }
    operator CB_u64() { CB_u64 i; i.v = v; return i; }

    std::string toS() const { return std::to_string(v); }
};
CB_Type CB_int::type = CB_Type();

// generic unsigned int - stored as u64 but can be implicitly casted to any integer type
struct CB_uint {
    static CB_Type type;
    uint64_t v;
    operator CB_i8() { CB_i8 i; i.v = v; return i; }
    operator CB_i16() { CB_i16 i; i.v = v; return i; }
    operator CB_i32() { CB_i32 i; i.v = v; return i; }
    operator CB_i64() { CB_i64 i; i.v = v; return i; }

    operator CB_u8() { CB_u8 i; i.v = v; return i; }
    operator CB_u16() { CB_u16 i; i.v = v; return i; }
    operator CB_u32() { CB_u32 i; i.v = v; return i; }
    operator CB_u64() { CB_u64 i; i.v = v; return i; }

    std::string toS() const { return std::to_string(v); }
};
CB_Type CB_uint::type = CB_Type();

// generic float - stored as f64 but can be implicitly casted to any floating point type
struct CB_float {
    static CB_Type type;
    double v;
    operator CB_f32() { CB_f32 i; i.v = v; return i; }
    operator CB_f64() { CB_f64 i; i.v = v; return i; }

    std::string toS() const { return std::to_string(v); }
};
CB_Type CB_float::type = CB_Type();




// owning pointer - owns the object and deallocates it when done.
template<typename T> // T is a CB type
struct CB_owning_pointer {
    static CB_Type type;
    T* v = nullptr;

    void destroy() {
        if (v != nullptr) {
            v->~T();
            free(v);
        }
        v = nullptr;
    }

    std::string toS() const {
        std::ostringstream oss;
        oss << "pointer!(" << std::hex << v << ")";
        return oss.str;
    }

    // default constructor
    CB_owning_pointer() {}
    ~CB_owning_pointer() { destroy(); }

    T* operator->() { ASSERT(v != nullptr); return v; }
    T& operator*() { ASSERT(v != nullptr); return *v; }

    // copy. FIXME: decide if deep copies are allowed or not.
    // CB_owning_pointer& operator=(const CB_owning_pointer& ptr) = delete;
    // CB_owning_pointer(const CB_owning_pointer& ptr) = delete;
    CB_owning_pointer& operator=(const CB_owning_pointer& ptr) {
        destroy();
        if (ptr.v != nullptr) {
            v = malloc(sizeof(T));
            new (v) T(*ptr.v);
        }
    }
    CB_owning_pointer(const CB_owning_pointer& ptr) { *this = ptr; }

    CB_owning_pointer& operator=(const T& t) {
        destroy();
        v = (T*)malloc(sizeof(T));
        new (v) T(t);
    }
    CB_owning_pointer(const T& t) { *this = t; }

    // move
    CB_owning_pointer& operator=(CB_owning_pointer&& ptr) {
        destroy();
        v = ptr.v;
        ptr.v = nullptr;
    }
    CB_owning_pointer(CB_owning_pointer&& ptr) { *this = std::move(ptr); }

    CB_owning_pointer& operator=(T&& t) {
        destroy();
        v = (T*)malloc(sizeof(T));
        new (v) T(std::move(t));
    }
    CB_owning_pointer(T&& t) { *this = std::move(t); }

};
template<typename T>
CB_Type CB_owning_pointer<T>::type = CB_Type();


// sharing pointer - never deallocates the object
// shares happily with other sharing pointers
// can not be created from an object, only from other pointers
template<typename T> // T is a CB type
struct CB_sharing_pointer {
    static CB_Type type;
    T* v = nullptr;

    void destroy() {
        v = nullptr;
    }

    std::string toS() const {
        std::ostringstream oss;
        oss << "pointer(" << std::hex << v << ")";
        return oss.str;
    }

    // default constructor
    CB_sharing_pointer() {}
    ~CB_sharing_pointer() { destroy(); }

    T* operator->() { ASSERT(v != nullptr); return v; }
    T& operator*() { ASSERT(v != nullptr); return *v; }

    // copy
    CB_sharing_pointer& operator=(const CB_sharing_pointer& ptr) { v = ptr.v; }
    CB_sharing_pointer(const CB_sharing_pointer& ptr) { *this = ptr; }

    CB_sharing_pointer& operator=(const CB_owning_pointer<T>& ptr) { v = ptr.v; }
    CB_sharing_pointer(const CB_owning_pointer<T>& ptr) { *this = ptr; }

    // move
    CB_sharing_pointer& operator=(CB_sharing_pointer&& ptr) { v = ptr.v; }
    CB_sharing_pointer(CB_sharing_pointer&& ptr) { *this = std::move(ptr); }
};
template<typename T>
CB_Type CB_sharing_pointer<T>::type = CB_Type();



// Derived types
struct CB_range {
    static CB_Type type;
    CB_f64 start;
    CB_f64 end;

    std::string toS() const {
        return start.toS() + ".." + end.toS();
    }
};
CB_Type CB_range::type = CB_Type();



template<int i=0> // fun workaround for enabling 0 argument template function
constexpr size_t sizeof_struct() {
    return 0;
}
template<typename T, typename... Rest>
constexpr size_t sizeof_struct() {
    return sizeof_struct<Rest...>() + sizeof(T);
}


struct Metadata {
    CB_Dynamic_seq<CB_String> member_ids;
    CB_Dynamic_seq<CB_bool> member_initialized;
};

// TODO: memory alignment (possibly turn on/off?)
template<typename... Members> // Members are all CB types
struct CB_struct {
    static CB_Type type;
    CB_owning_pointer<Metadata> metadata;
    char v[sizeof_struct<Members...>()];

    destroy() {
        if (sizeof...(Members) > 0) {
            delete_members<Members...>();
        }
    }

    std::string toS() const {
        return "struct(" + std::to_string(CB_struct<Members...>::type.uid) + ")"; // FIXME: better toS();
    }

    CB_struct(bool init=true) {
        metadata = Metadata();
        metadata->member_initialized.resize(sizeof...(Members));
        if (init) init_members<Members...>();
    }

    ~CB_struct() { destroy(); }

    template<typename T>
    T& get(int i) {
        return get<T, Members...>(i, 0);
    }

private:
    template<typename T, int dump=0> // fun workaround for enabling 0 argument template function
    T& get(int i, size_t offset_bytes){
        ASSERT(sizeof...(Members) == 0);
        ASSERT(false, "no elements to get from");
    }

    template<typename T, typename First>
    T& get(int i, size_t offset_bytes) {
        ASSERT(i == 0);
        ASSERT((std::is_same<T, First>::value)); // needs double parens inside the ASSERT macro for some reason
        ASSERT(offset_bytes+sizeof(T) == sizeof_struct());
        return *(T*)(((char*)v)+offset_bytes);
    }

    template<typename T, typename First, typename Next, typename... Rest>
    T& get(int i, size_t offset_bytes) {
        ASSERT(offset_bytes+sizeof(T) < sizeof_struct());
        if (i == 0) {
            ASSERT((std::is_same<T, First>::value)); // needs double parens inside the ASSERT macro for some reason
            return *(T*)(((char*)v)+offset_bytes);
        }
        else return get<T, Next, Rest...>(i-1, offset_bytes+sizeof(First));
    }

    template<int i=0>
    void delete_members(size_t offset_bytes=0) {
        ASSERT(offset_bytes == sizeof_struct());
    }

    template<typename T, typename... Rest>
    void delete_members(size_t offset_bytes=0) {
        ASSERT(offset_bytes+sizeof(T) <= sizeof_struct());
        T& t = *(T*)(((char*)v)+offset_bytes);
        t.~T();
        delete_members<Rest...>(offset_bytes+sizeof(T));
    }

    template<int i=0>
    void init_members(int index=0, size_t offset_bytes=0) {
        ASSERT(offset_bytes == sizeof_struct());
        ASSERT(index == sizeof...(Members));
    }

    template<typename T, typename... Rest>
    void init_members(int index=0, size_t offset_bytes=0) {
        ASSERT(offset_bytes+sizeof(T) <= sizeof_struct());
        ASSERT(index < sizeof...(Members));
        new ((T*)(((char*)v)+offset_bytes)) T(); // in place construction
        metadata->member_initialized.set(index, CB_bool(true));
        init_members<Rest...>(index+1, offset_bytes+sizeof(T));
    }

    constexpr size_t sizeof_struct() {
        return ::sizeof_struct<Members...>();
    }

};
template<typename... Members>
CB_Type CB_struct<Members...>::type = CB_Type();



// todo: function, operator


































/*
struct CB_any {
    static CB_Type type; // type any
    CB_Type v_type; // the type of v
    void* v_ptr = nullptr;

    void destroy() {
        ASSERT(false, "unable to deduce type of v_ptr (cause cpp sucks) -> unable to call the currect constructor");
        // if (v_ptr != nullptr) {
        //     ((T*)v_ptr)->destroy();
        // }
        free(v_ptr);
        v_ptr = nullptr;
    }

    template<typename T>
    std::string toS() const {
        if (v_ptr == nullptr) return "";
        ASSERT(T::type == v_type);
        return ((T*)v_ptr)->toS();
    }

    CB_any() {}
    ~CB_any() { destroy(); }

    template<typename T>
    T& value() {
        ASSERT(!(std::is_same<T, CB_any>::value));
        ASSERT(T::type == v_type);
        if (v_ptr == nullptr) allocate<T>();
        return *(T*)v_ptr;
    }

    template<typename T>
    CB_any& operator=(const T& t) {
        v_type = T::type;
        if (v_ptr != nullptr) {

        }
    }

    template<typename T>
    void allocate(bool init=true) {
        destroy();
        v_ptr = malloc(sizeof(T));
        if (init) new ((T*)v_ptr) T();
    }

};
*/