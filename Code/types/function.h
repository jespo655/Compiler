#pragma once

#include "type.h"
#include "primitives.h"
#include "string.h"
#include "dynamic_seq.h"
#include "pointers.h"

#include <string>

/*
Range - a compact data structure with a start and an end.
Can be iterated over

Syntax:
foo : fn(int, int)->(int, int) = fn(a: int, b: int)->(c: int, d: int) { return a, b; };
sum : fn(int, int)->int = fn(a: int, b: int)->int { return a+b; }; // only one return value -> don't need the paren
bar : fn() = fn() {}; // no return value -> don't need the arrow
*/

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
    CB_Owning_pointer<Function_metadata> metadata = alloc(Function_metadata());
    void (*v)() = nullptr; // function pointer

    std::string toS() {
        return "function"; // FIXME: better toS()
    }

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
        metadata = alloc<Function_metadata>(*fn.metadata);
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
    //     metadata = ???; // no way to separate in and out types
    //     return *this;
    // }
    // CB_Function(void (*fn)(In_types... ins, Out_types&... outs)) { *this = fn; }

    CB_Function& operator=(nullptr_t np) {
        ASSERT(np == nullptr);
        v = nullptr;
        metadata = alloc(Function_metadata());
        return *this;
    }
    CB_Function(const nullptr_t& fn) { *this = fn; }

    template<typename... Types>
    void set_in_args() {
        ASSERT(metadata != nullptr);
        metadata->in_args.clear();
        add_in_args<Types...>();
    }

    template<typename... Types>
    void set_out_args() {
        metadata->out_args.clear();
        add_out_args<Types...>();
    }

private:

    template<typename T, typename... Rest>
    void add_in_args() {
        Function_arg arg;
        arg.id = "_arg_" + metadata->in_args.size;
        arg.type = T::type;
        metadata->in_args.add(arg);
        add_in_args<Rest...>();
    }
    template<int i=0> add_in_args() {}

    template<typename T, typename... Rest>
    void add_out_args() {
        Function_arg arg;
        arg.id = "_retval_" + metadata->out_args.size;
        arg.type = T::type;
        metadata->out_args.add(arg);
        add_out_args<Rest...>();
    }
    template<int i=0> add_out_args() {}

    template<int index>
    void assert_out_types() {
        ASSERT(index == metadata->out_args.size, "The number of out arguments doesn't match the function metadata.");
    }

    template<int index, typename T, typename... Rest>
    void assert_out_types() {
        ASSERT(metadata != nullptr);
        ASSERT(index < metadata->out_args.size, "The number of out arguments doesn't match the function metadata.");
        const CB_Type& T1 = metadata->out_args[index].type;
        const CB_Type& T2 = get_type(T());
        ASSERT(T1 == T2, "Type mismatch at out type "+std::to_string(index)+", "+T1.toS()+" and "+T2.toS());
        assert_out_types<index+1, Rest...>();
    }

    template<int index>
    void assert_in_types() {
        ASSERT(index == metadata->in_args.size, "The number of in arguments doesn't match the function metadata.");
        ASSERT(0 == metadata->out_args.size);
    }

    template<int index, typename T, typename... Rest>
    void assert_in_types() {
        ASSERT(metadata != nullptr);
        if (index == metadata->in_args.size) {
            return assert_out_types<0, T, Rest...>();
        }
        ASSERT(index < metadata->in_args.size, "The number of in arguments doesn't match the function metadata.");
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

};
CB_Type CB_Function::type = CB_Type("fn");


