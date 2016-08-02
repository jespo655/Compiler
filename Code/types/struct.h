#pragma once

#include "type.h"
#include "pointers.h"
#include "any.h"
#include <string>


/*
NOTE: the struct system has to be remade - it's impossible to create templated types
from run time information and that must happen all the time in the parser.


*/

struct Struct_member {
    CB_String id;
    CB_Type type;
    CB_Any default_value;
    CB_Bool initialized;
};

struct Struct_metadata {
    CB_Dynamic_seq<Struct_member> members;
};

struct Struct {
    CB_Type type;
    CB_Owning_pointer<Struct_metadata> metadata;
    CB_Dynamic_seq<CB_Any> values;

    void init_members() {
        ASSERT(metadata != nullptr);
        values.clear();
        for (auto& member : metadata->members) {

        }
    }
};







/*




// helper functions for determining the size of the struct
template<int i=0> // fun workaround for enabling 0 argument template function
constexpr size_t total_size() {
    return 0;
}
template<typename T, typename... Rest>
constexpr size_t total_size() {
    return total_size<Rest...>() + sizeof(T);
}



// template<int N, typename... Types>
// using type_with_index = typename std::tuple_element<N, std::tuple<Types...>>::type;

struct Struct_member {
    CB_String id;
    CB_Type type;
    CB_Bool has_default_value;
    CB_Bool initialized;
};

struct Struct_metadata {
    CB_Dynamic_seq<Struct_member> members;
};

// TODO: memory alignment (possibly turn on/off?)
// Maybe: uid as template, have metadata static
template<typename... Members> // Members are all CB types
struct CB_Struct {

    static CB_Type type;
    CB_Owning_pointer<Struct_metadata> metadata;
    char v[total_size<Members...>()];

    std::string toS() const {
        return "struct(" + std::to_string(CB_Struct<Members...>::type.uid) + ")"; // FIXME: better toS();
    }

    CB_Struct(bool init=true) {
        metadata = alloc(Struct_metadata());
        metadata->members.resize(sizeof...(Members));
        if (init) init_members<Members...>();
    }

    ~CB_Struct() {
        if (sizeof...(Members) > 0) {
            delete_members<Members...>();
        }
    }

    CB_Struct& operator=(const CB_Struct& s) {
        assign_members(s);
        metadata = alloc<Struct_metadata>(*s.metadata);
        return *this;
    }
    CB_Struct(const CB_Struct& s) { *this = s; }

    CB_Struct& operator=(CB_Struct&& s) {
        // swap v arrays
        constexpr size_t size = total_size<Members...>();
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
        ASSERT(offset_bytes+sizeof(T) == total_size<Members...>());
        return *(T*)(((char*)v)+offset_bytes);
    }

    template<typename T, typename First, typename Next, typename... Rest>
    T& get(int i, size_t offset_bytes) {
        ASSERT(offset_bytes+sizeof(T) < total_size<Members...>());
        if (i == 0) {
            ASSERT((std::is_same<T, First>::value)); // needs double parens inside the ASSERT macro for some reason
            return *(T*)(((char*)v)+offset_bytes);
        }
        else return get<T, Next, Rest...>(i-1, offset_bytes+sizeof(First));
    }

    template<int i=0>
    void delete_members(size_t offset_bytes=0) {
        ASSERT(offset_bytes == total_size<Members...>());
    }

    template<typename T, typename... Rest>
    void delete_members(size_t offset_bytes=0) {
        ASSERT(offset_bytes+sizeof(T) <= total_size<Members...>());
        T& t = *(T*)(((char*)v)+offset_bytes);
        t.~T();
        delete_members<Rest...>(offset_bytes+sizeof(T));
    }

    template<int i=0>
    void init_members(int index=0, size_t offset_bytes=0) {
        ASSERT(offset_bytes == total_size<Members...>());
        ASSERT(index == sizeof...(Members));
    }

    template<typename T, typename... Rest>
    void init_members(int index=0, size_t offset_bytes=0) {
        ASSERT(offset_bytes+sizeof(T) <= total_size<Members...>());
        ASSERT(index < sizeof...(Members));
        new ((T*)(((char*)v)+offset_bytes)) T(); // in place construction
        metadata->members[index].initialized = CB_Bool(true);
        init_members<Rest...>(index+1, offset_bytes+sizeof(T));
    }


    template<int i=0>
    void assign_members(CB_Struct s, int index=0, size_t offset_bytes=0) {
        ASSERT(offset_bytes == total_size<Members...>());
        ASSERT(index == sizeof...(Members));
    }

    template<typename T, typename... Rest>
    void assign_members(CB_Struct s, int index=0, size_t offset_bytes=0) {
        ASSERT(offset_bytes+sizeof(T) <= total_size<Members...>());
        ASSERT(index < sizeof...(Members));
        *(T*)(((char*)v)+offset_bytes) = *(T*)(((char*)s.v)+offset_bytes); // copy assignment
        metadata->members[index].initialized = CB_Bool(true);
        assign_members<index+1, Rest...>(offset_bytes+sizeof(T));
    }




};
template<typename... Members>
CB_Type CB_Struct<Members...>::type = CB_Type("struct");


*/