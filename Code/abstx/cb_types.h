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



Built in functions that refer to types:
* sizeof(Type t)

*/





struct CB_Type
{
    static CB_Type type; // self reference / CB_Type
    int uid = get_unique_id();
    std::string name = ""; // only compile time

    CB_Type() {}
    CB_Type(const std::string& name) : name{name} {}

    std::string toS() const {
        if (name == "") return "type("+std::to_string(uid)+")";
        return name;
    }

    bool operator==(const CB_Type& o) const { return uid == o.uid; }
    bool operator!=(const CB_Type& o) const { return !(*this==o); }
};
CB_Type CB_Type::type = CB_Type("type");


template<typename T> // T is a CB type
struct CB_Dynamic_seq {
    static CB_Type type;
    constexpr static CB_Type& member_type = T::type;
    constexpr static bool dynamic = true;
    int size = 0;
    int capacity = 16; // arbitrary power of 2
    T* v_ptr = nullptr;
    int id = get_unique_id();

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

    ~CB_Dynamic_seq() {
        for (int i = 0; i < size; ++i) {
            v_ptr[i].~T();
        }
        free(v_ptr);
        v_ptr = nullptr;
        size = 0;
        capacity = 0;
    }

    // copy
    CB_Dynamic_seq& operator=(const CB_Dynamic_seq& seq) {
        for (int i = 0; i < size; ++i) {
            v_ptr[i].~T();
        }
        resize(seq.size, false);
        for (int i = 0; i < size; ++i) {
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
                new (&v_ptr[it]) T(); // fill with default values
            }
            new (&v_ptr[i]) T(t);
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
                for (int i = size; i < new_size; ++i) {
                    new (&v_ptr[i]) T(default_value); // fill with default values
                }
            }
        } else {
            // properly delete everything outside the new size
            for (int i = new_size; i < size; ++i) v_ptr[i].~T();
        }
        size = new_size;
    }

    void clear() {
        // properly delete cleared things
        for (int i = 0; i < size; ++i)
            v_ptr[i].~T();
        size = 0;
    }

    void reallocate(int capacity, bool move=true) {
        this->capacity = capacity;
        T* new_ptr = (T*)malloc(capacity*sizeof(T));
        ASSERT(new_ptr != nullptr);
        if (capacity < size) {
            for (int i = capacity; i < size; ++i) v_ptr[i].~T(); // properly delete the Ts we won't move
            size = capacity;
        }
        if (move) memcpy(new_ptr, v_ptr, size*sizeof(T)); // move the values
        else for (int i = 0; i < size; ++i) v_ptr[i].~T(); // properly delete all Ts we don't move
        free(v_ptr);
        v_ptr = new_ptr;
    }

    struct iterator {
        int index = 0;
        const CB_Dynamic_seq& seq;
        iterator(const CB_Dynamic_seq& seq, int i=0) : seq{seq}, index{i} {}
        const T& operator*() const { ASSERT(index < seq.size); return seq.v_ptr[index].type; }
        iterator& operator++() { ++index; return *this; }
        bool operator==(const iterator& o) const { return index == o.index; }
        bool operator!=(const iterator& o) const { return !(*this == o); }
        bool operator<(const iterator& o) const { return index < o.index; }
    };

    iterator begin() const { return iterator(*this); }
    iterator end() const { return iterator(*this, size); }
};

template<typename T>
CB_Type CB_Dynamic_seq<T>::type = CB_Type(T::type.toS()+"[]");


template<typename T, int c_size> // T is a CB type
struct CB_Static_seq {
    static CB_Type type;
    constexpr static CB_Type& member_type = T::type;
    constexpr static bool dynamic = false;
    constexpr static int size = c_size;
    constexpr static int capacity = c_size;
    T v[c_size];

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
    ~CB_Static_seq() {
        for (int i = 0; i < size; ++i) v[i].~T();
    }

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

    struct iterator {
        int index = 0;
        const CB_Static_seq& seq;
        iterator(const CB_Static_seq& seq, int i=0) : seq{seq}, index{i} {}
        const T& operator*() const { ASSERT(index < seq.size); return seq.v_ptr[index]; }
        iterator& operator++() { ++index; return *this; }
        bool operator==(const iterator& o) const { return index == o.index; }
        bool operator!=(const iterator& o) const { return !(*this == o); }
        bool operator<(const iterator& o) const { return index < o.index; }
    };

    iterator begin() const { return iterator(*this); }
    iterator end() const { return iterator(*this, size); }
};

template<typename T, int size>
CB_Type CB_Static_seq<T, size>::type = CB_Type(T::type.toS()+"["+std::to_string(size)+"]");


// string: just like a dynamic array of characters
// actual allocated size is size+1 - the last character is always '\0'
// operating on individual chars is not allowed.
struct CB_String {
    static CB_Type type;
    int size = 0; // size in bytes, not in UTF-8 characters
    int capacity = 0;
    char* v_ptr = nullptr;

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
    ~CB_String() {
        free(v_ptr);
        v_ptr = nullptr;
    }

    bool operator==(const CB_String& str) const { return size == str.size && strcmp(v_ptr, str.v_ptr); }
    bool operator!=(const CB_String& str) const { !(*this == str); }

    // copy
    CB_String& operator=(const CB_String& str) {
        if (capacity < str.size)
            reallocate(str.size, false);
        ASSERT(capacity >= str.size);
        memcpy(v_ptr, str.v_ptr, size+1);
        size = str.size;
        return *this;
    }
    CB_String(const CB_String& str) { *this = str; }

    // move
    CB_String& operator=(CB_String&& str) {
        free(v_ptr);
        size = str.size;
        v_ptr = str.v_ptr;
        str.v_ptr = nullptr;
        return *this;
    }
    CB_String(CB_String&& str) { *this = std::move(str); }

    void reallocate(int capacity, bool move=true) {
        this->capacity = capacity;
        char* new_ptr = (char*)malloc(capacity+1);
        ASSERT(new_ptr != nullptr);
        if (capacity < size) size = capacity;
        if (move) memcpy(new_ptr, v_ptr, size+1);
        free(v_ptr);
        v_ptr = new_ptr;
    }
};

CB_Type CB_String::type = CB_Type("string");


struct CB_Bool {
    static CB_Type type;
    bool v = false;
    CB_Bool() {}
    CB_Bool(bool b) { this->v = b; }
    std::string toS() const { return v? "true" : "false"; }
};
CB_Type CB_Bool::type = CB_Type("bool");


#define CB_NUMBER_TYPE(CB_t, CPP_t)                         \
struct CB_##CB_t {                                          \
    static CB_Type type;                                    \
    CPP_t v = 0;                                            \
    CB_##CB_t() {}                                          \
    CB_##CB_t(const CPP_t& v) { this->v = v; }              \
    std::string toS() const { return std::to_string(v); }   \
};                                                          \
CB_Type CB_##CB_t::type = CB_Type(#CB_t);

CB_NUMBER_TYPE(i8, int8_t);
CB_NUMBER_TYPE(i16, int16_t);
CB_NUMBER_TYPE(i32, int32_t);
CB_NUMBER_TYPE(i64, int64_t);

CB_NUMBER_TYPE(u8, uint8_t);
CB_NUMBER_TYPE(u16, uint16_t);
CB_NUMBER_TYPE(u32, uint32_t);
CB_NUMBER_TYPE(u64, uint64_t);

CB_NUMBER_TYPE(f32, float);
CB_NUMBER_TYPE(f64, double);

// generic int - stored as i64 but can be implicitly casted to any integer type
struct CB_Int {
    static CB_Type type;
    int64_t v;
    CB_Int() {}
    CB_Int(const int64_t& v) { this->v = v; }
    std::string toS() const { return std::to_string(v); }
    operator CB_i8() { CB_i8 i; i.v = v; return i; }
    operator CB_i16() { CB_i16 i; i.v = v; return i; }
    operator CB_i32() { CB_i32 i; i.v = v; return i; }
    operator CB_i64() { CB_i64 i; i.v = v; return i; }

    operator CB_u8() { CB_u8 i; i.v = v; return i; }
    operator CB_u16() { CB_u16 i; i.v = v; return i; }
    operator CB_u32() { CB_u32 i; i.v = v; return i; }
    operator CB_u64() { CB_u64 i; i.v = v; return i; }
};
CB_Type CB_Int::type = CB_Type("int");

// generic unsigned int - stored as u64 but can be implicitly casted to any integer type
struct CB_Uint {
    static CB_Type type;
    uint64_t v;
    CB_Uint() {}
    CB_Uint(const uint64_t& v) { this->v = v; }
    std::string toS() const { return std::to_string(v); }
    operator CB_i8() { CB_i8 i; i.v = v; return i; }
    operator CB_i16() { CB_i16 i; i.v = v; return i; }
    operator CB_i32() { CB_i32 i; i.v = v; return i; }
    operator CB_i64() { CB_i64 i; i.v = v; return i; }

    operator CB_u8() { CB_u8 i; i.v = v; return i; }
    operator CB_u16() { CB_u16 i; i.v = v; return i; }
    operator CB_u32() { CB_u32 i; i.v = v; return i; }
    operator CB_u64() { CB_u64 i; i.v = v; return i; }
};
CB_Type CB_Uint::type = CB_Type("uint");

// generic float - stored as f64 but can be implicitly casted to any floating point type
struct CB_Float {
    static CB_Type type;
    double v;
    CB_Float() {}
    CB_Float(const double& v) { this->v = v; }
    std::string toS() const { return std::to_string(v); }
    operator CB_f32() { CB_f32 i; i.v = v; return i; }
    operator CB_f64() { CB_f64 i; i.v = v; return i; }
};
CB_Type CB_Float::type = CB_Type("float");




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

    // copy - not allowed. If passing a owned_ptr to a function, move constructor must be used. (In CB this will be done automatically)
    CB_Owning_pointer& operator=(const CB_Owning_pointer& ptr) = delete;
    CB_Owning_pointer(const CB_Owning_pointer& ptr) = delete;

    CB_Owning_pointer& operator=(const T& t) {
        this->~CB_Owning_pointer();
        v = (T*)malloc(sizeof(T));
        new (v) T(t);
        return *this;
    }
    CB_Owning_pointer(const T& t) { *this = t; }

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

    CB_Owning_pointer& operator=(T&& t) {
        this->~CB_Owning_pointer();
        v = (T*)malloc(sizeof(T));
        new (v) T(std::move(t));
        return *this;
    }
    CB_Owning_pointer(T&& t) { *this = std::move(t); }

    CB_Owning_pointer& operator=(T*&& ptr) {
        this->~CB_Owning_pointer();
        v = ptr;
        ptr = nullptr;
        return *this;
    }
    CB_Owning_pointer(T*&& ptr) { *this = std::move(ptr); }
};
template<typename T>
CB_Type CB_Owning_pointer<T>::type = CB_Type(T::type.toS()+"*!");


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

    T* operator->() { ASSERT(v != nullptr); return v; }
    T& operator*() { ASSERT(v != nullptr); return *v; }

    // copy
    CB_Sharing_pointer& operator=(const CB_Sharing_pointer& ptr) { v = ptr.v; return *this; }
    CB_Sharing_pointer(const CB_Sharing_pointer& ptr) { *this = ptr; }

    CB_Sharing_pointer& operator=(const CB_Owning_pointer<T>& ptr) { v = ptr.v; return *this; }
    CB_Sharing_pointer(const CB_Owning_pointer<T>& ptr) { *this = ptr; }

    CB_Sharing_pointer& operator=(const T*& ptr) { v = ptr; return *this; }
    CB_Sharing_pointer(const T*& ptr) { *this = ptr; }

    CB_Sharing_pointer& operator=(const nullptr_t& ptr) { ASSERT(ptr == nullptr); v = ptr; return *this; }
    CB_Sharing_pointer(const nullptr_t& ptr) { *this = ptr; }
};
template<typename T>
CB_Type CB_Sharing_pointer<T>::type = CB_Type(T::type.toS()+"*");

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



// Derived types
struct CB_Range {
    static CB_Type type;
    CB_f64 start;
    CB_f64 end;

    std::string toS() const {
        return start.toS() + ".." + end.toS();
    }
};
CB_Type CB_Range::type = CB_Type("range");



template<int i=0> // fun workaround for enabling 0 argument template function
constexpr size_t sizeof_struct() {
    return 0;
}
template<typename T, typename... Rest>
constexpr size_t sizeof_struct() {
    return sizeof_struct<Rest...>() + sizeof(T);
}



template<int N, typename... Types>
using type_with_index = typename std::tuple_element<N, std::tuple<Types...>>::type;



struct Struct_metadata {
    CB_Dynamic_seq<CB_String> member_ids;
    CB_Dynamic_seq<CB_Bool> member_initialized;
};

// TODO: memory alignment (possibly turn on/off?)
// Maybe: uid as template, have metadata static
template<typename... Members> // Members are all CB types
struct CB_Struct {
    static CB_Type type;
    CB_Owning_pointer<Struct_metadata> metadata;
    char v[sizeof_struct<Members...>()];

    std::string toS() const {
        return "struct(" + std::to_string(CB_Struct<Members...>::type.uid) + ")"; // FIXME: better toS();
    }

    CB_Struct(bool init=true) {
        metadata = Struct_metadata();
        metadata->member_initialized.resize(sizeof...(Members));
        if (init) init_members<Members...>();
    }

    ~CB_Struct() {
        if (sizeof...(Members) > 0) {
            delete_members<Members...>();
        }
    }

    // Deep copies are hard to do properly, since we cannot effectively iterate through template pack types.
    // For now, this operator is not allowed.
    CB_Struct& operator=(const CB_Struct& s) = delete;
    CB_Struct(const CB_Struct& s) = delete;

    // CB_Struct& operator=(const CB_Struct& s) {
    //     ~CB_Struct();
    //     metadata = *s.metadata;
    //     int offset = 0;
    //     for (int i = 0; i < sizeof...(Members); ++i) {
    //         typedef type_with_index<i, Members...> T; // error: i not consntexpr
    //         *(T*)(v+offset) = *(T*)(s.v+offset);
    //         offset += sizeof(T);
    //     }
    //     return *this;
    // }
    // CB_Struct(const CB_Struct& s) { *this = s; }

    CB_Struct& operator=(CB_Struct&& s) {
        // swap v arrays
        constexpr size_t size = sizeof_struct();
        char buffer[size];
        memcpy(buffer, v, size);
        memcpy(v, s.v, size);
        memcpy(s.v, buffer, size);
        metadata = std::move(s.metadata);
        return *this;
    }
    CB_Struct(CB_Struct&& s) { *this = s; }

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
        metadata->member_initialized.set(index, CB_Bool(true));
        init_members<Rest...>(index+1, offset_bytes+sizeof(T));
    }

    constexpr size_t sizeof_struct() {
        return ::sizeof_struct<Members...>();
    }

};
template<typename... Members>
CB_Type CB_Struct<Members...>::type = CB_Type("struct");







// todo: generic function, operator



struct Function_arg {
    CB_String id;
    CB_Type type;
    CB_Bool has_default_value;
};

struct Function_metadata {
    CB_Dynamic_seq<Function_arg> in_args;
    CB_Dynamic_seq<Function_arg> out_args;
};

struct CB_Function
{
    static CB_Type type;
    CB_Owning_pointer<Function_metadata> metadata = Function_metadata();
    void (*v)() = nullptr; // function pointer

    template<typename... Types>
    void operator()(Types... args)
    {
        ASSERT(v != nullptr);
        ASSERT(metadata != nullptr);
        assert_types<Types...>();
        auto fn_ptr = (void (*)(Types...))v;
        (*fn_ptr)(args...);
    }

    CB_Function() {}
    ~CB_Function() {}

    CB_Function& operator=(const CB_Function& fn) {
        v = fn.v;
        metadata = *fn.metadata;
        return *this;
    }
    CB_Function(const CB_Function& fn) { *this = fn; }

    CB_Function& operator=(CB_Function&& fn) {
        v = fn.v;
        metadata = std::move(fn.metadata);
        return *this;
    }
    CB_Function(CB_Function&& fn) { *this = fn; }

    // CB_Function& operator=(void (*fn)(In_types... ins, Out_types&... outs)) {
    //     v = fn;
    //     metadata = ???;
    //     return *this;
    // }
    // CB_Function(void (*fn)(In_types... ins, Out_types&... outs)) { *this = fn; }

    CB_Function& operator=(nullptr_t np) {
        ASSERT(np == nullptr);
        v = nullptr;
        metadata = Function_metadata();
        return *this;
    }
    CB_Function(const nullptr_t& fn) { *this = fn; }

    template<typename... Types>
    void set_in_args() {
        ASSERT(metadata != nullptr);
        metadata->in_args.clear();
        add_in_args<Types...>();
    }

    template<typename T, typename... Rest>
    void add_in_args() {
        Function_arg arg;
        arg.id = "_arg_" + metadata->in_args.size;
        arg.type = T::type;
        metadata->in_args.add(arg);
        add_in_args<Rest...>();
    }
    template<int i=0> add_in_args() {}

    template<typename... Types>
    void set_out_args() {
        metadata->out_args.clear();
        add_out_args<Types...>();
    }

    template<typename T, typename... Rest>
    void add_out_args() {
        Function_arg arg;
        arg.id = "_retval_" + metadata->out_args.size;
        arg.type = T::type;
        metadata->out_args.add(arg);
        add_out_args<Rest...>();
    }
    template<int i=0> add_out_args() {}

private:

    template<int index>
    void assert_out_types() {
        ASSERT(index == metadata->out_args.size, "The number of arguments doesn't match the function metadata.");
    }

    template<int index, typename T, typename... Rest>
    void assert_out_types() {
        ASSERT(metadata != nullptr);
        ASSERT(index < metadata->out_args.size, "The number of arguments doesn't match the function metadata.");
        const CB_Type& T1 = metadata->out_args[index].type;
        const CB_Type& T2 = get_type(T());
        ASSERT(T1 == T2, "Type mismatch at out type "+std::to_string(index)+", "+T1.toS()+" and "+T2.toS());
        assert_out_types<index+1, Rest...>();
    }

    template<int index>
    void assert_in_types() {
        ASSERT(index == metadata->in_args.size, "The number of arguments doesn't match the function metadata.");
        ASSERT(0 == metadata->out_args.size);
    }

    template<int index, typename T, typename... Rest>
    void assert_in_types() {
        ASSERT(metadata != nullptr);
        if (index == metadata->in_args.size) {
            return assert_out_types<0, T, Rest...>();
        }
        ASSERT(index < metadata->in_args.size, "The number of arguments doesn't match the function metadata.");
        const CB_Type& T1 = metadata->in_args[index].type;
        const CB_Type& T2 = get_type(T());
        ASSERT(T1 == T2, "Type mismatch at in type "+std::to_string(index)+", "+T1.toS()+" and "+T2.toS());
        assert_in_types<index+1, Rest...>();
    }

    template<typename... Types>
    void assert_types() {
        if (sizeof...(Types) == 0) return;
        return assert_in_types<0, Types...>();
    }

    template<typename T, typename Type=CB_Type*, Type=&T::type>
    const CB_Type& get_type(const T& object) { return T::type; }

    template<typename T, typename Type=CB_Type*, Type=&T::type>
    const CB_Type& get_type(const T* pointer) { return T::type; }

    // template<typename T, typename Type=CB_Type*, Type=&T::type>
    // bool types_match(const CB_Type& type, const T& instance) {
    //     return type == T::type;
    // }

    // template<typename T, typename Type=CB_Type*, Type=&T::type>
    // bool types_match(const CB_Type& type, T* instance) {
    //     return type == T::type;
    // }

};
CB_Type CB_Function::type = CB_Type("fn");







struct Generic_arg : Function_arg {
    CB_Int generic_index = -1;
};

struct Generic_arg_list_metadata {
    CB_Dynamic_seq<Generic_arg> member_ids;
};

struct Generic_function_metadata {
    CB_Dynamic_seq<Generic_arg> in;
    CB_Dynamic_seq<Generic_arg> out;
};



struct CB_Generic_function
{
    static CB_Type type;
    CB_Owning_pointer<Generic_function_metadata> metadata;
    CB_Dynamic_seq<CB_Function> constructed_fns;
    CB_Generic_function() {}
    ~CB_Generic_function() {}

    // CB_Generic_function& operator=(const CB_Generic_function& fn) {
    //     v = fn.v;
    //     metadata = *fn.metadata;
    //     return *this;
    // }
    // CB_Generic_function(const CB_Generic_function& fn) { *this = fn; }

    // CB_Generic_function& operator=(CB_Generic_function&& fn) {
    //     v = fn.v;
    //     metadata = std::move(fn.metadata);
    //     return *this;
    // }
    // CB_Generic_function(CB_Generic_function&& fn) { *this = fn; }

    // CB_Generic_function& operator=(void (*fn)(In_types... ins, Out_types&... outs)) {
    //     v = fn;
    //     metadata = ???;
    //     return *this;
    // }
    // CB_Generic_function(void (*fn)(In_types... ins, Out_types&... outs)) { *this = fn; }

    // CB_Generic_function& operator=(const nullptr_t& fn) { ASSERT(fn == nullptr); v = fn; return *this; }
    // CB_Generic_function(const nullptr_t& fn) { *this = fn; }

    // TODO:
    // template<typename... Types>
    // void operator()(Types... args)
    // {
    //     ASSERT(v != nullptr);
    //     auto fn_ptr = (void (*)(Types...))v;
    //     (*fn_ptr)(args...);
    // }

};
CB_Type CB_Generic_function::type = CB_Type("generic_fn");





struct CB_Operator
{
    static CB_Type type;
    CB_Owning_pointer<Generic_function_metadata> metadata;
    CB_Dynamic_seq<CB_Function> constructed_fns;
    CB_Operator() {}
    ~CB_Operator() {}
};
CB_Type CB_Operator::type = CB_Type("operator");








/*
struct CB_any {
    static CB_Type type; // type any
    CB_Type v_type; // the type of v
    void* v_ptr = nullptr;

    template<typename T>
    std::string toS() const {
        if (v_ptr == nullptr) return "";
        ASSERT(T::type == v_type);
        return ((T*)v_ptr)->toS();
    }

    CB_any() {}
    ~CB_any() {
        ASSERT(false, "unable to deduce type of v_ptr (cause cpp sucks) -> unable to call the currect constructor");
        // if (v_ptr != nullptr) {
        //     ((T*)v_ptr)->~T();
        // }
        free(v_ptr);
        v_ptr = nullptr;
    }

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
        ~CB_any();
        v_ptr = malloc(sizeof(T));
        if (init) new ((T*)v_ptr) T();
    }

};
*/