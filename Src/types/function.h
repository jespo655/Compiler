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
foo : fn(int, int)->(int, int);
sum : fn(int, int)->int;            // only one return value -> don't need the paren
bar : fn();                         // no return value -> don't need the arrow
*/

struct Function_type : CB_Type
{
    CB_Dynamic_seq<CB_Sharing_pointer<CB_Type>> in_types;
    CB_Dynamic_seq<CB_Sharing_pointer<CB_Type>> out_types;

    operator CB_Type() { return *this; }

    std::string toS() const override {
        std::ostringstream oss;

        oss << "fn(";
        for (int i = 0; i < in_types.size; ++i) {
            if (i > 0) oss << ", ";
            oss << in_types[i].toS();
        }
        oss << ")";
        if (out_types.size == 0) return oss.str();

        oss << "->";
        if (out_types.size > 1) oss << "(";
        for (int i = 0; i < out_types.size; ++i) {
            if (i > 0) oss << ", ";
            oss << out_types[i].toS();
        }
        if (out_types.size > 1) oss << ")";
        return oss.str();
    }

    bool operator==(const Function_type& o) const { return o.in_types == in_types && o.out_types == out_types; }
    bool operator!=(const Function_type& o) const { return !(*this==o); }
    bool operator==(const CB_Type& o) const { toS() == o.toS(); }
    bool operator!=(const CB_Type& o) const { return !(*this==o); }

};


























/*


struct Function_arg {
    CB_String id;
    CB_Type type;
    CB_Any default_value;
    CB_Bool is_using = false; // only for structs - imports that struct's members into the function scope
    CB_Bool has_default_value = false;
    CB_Bool explicit_uninitialized = false;

    Function_arg() {};
    Function_arg(const CB_String& id, const CB_Type& type, const CB_Any& default_value, bool is_using=false)
        : id{id}, type{type}, default_value{default_value}, is_using{is_using} {};

    std::string toS() const { return "Function_arg(" + id.toS() + ":" + type.toS() + ")"; }
};

struct Function_metadata {
    CB_Dynamic_seq<Function_arg> in_args;
    CB_Dynamic_seq<Function_arg> out_args;

    // Constructors has to be speficied, otherwise the default move constructor is used when we want to copy
    Function_metadata() {};
    Function_metadata(const Function_metadata& fm) { *this = fm; }
    Function_metadata(Function_metadata&& fm) { *this = std::move(fm); }
    Function_metadata& operator=(const Function_metadata& fm) { in_args = fm.in_args; out_args = fm.out_args; }
    Function_metadata& operator=(Function_metadata&& fm) { in_args = std::move(fm.in_args); out_args = std::move(fm.out_args); }
    ~Function_metadata() {}
};

struct Function_type : CB_Type
{
    CB_Owning_pointer<Function_metadata> metadata = alloc<Function_metadata>(Function_metadata());

    Function_type() : CB_Type{} {}

    operator CB_Type() { return *this; }

    std::string toS() const override {
        ASSERT(metadata != nullptr);
        std::ostringstream oss;

        oss << "fn(";
        for (int i = 0; i < metadata.in_args.size; ++i) {
            Function_arg& arg = metadata->in_args[i];
            if (i > 0) oss << ", ";
            if (arg.is_using) oss << "using ";
            ASSERT(arg.id.size > 0);
            oss << arg.id.toS() << ":" << arg.type.toS();
            if (arg.explicit_uninitialized) { ASSERT(arg.has_default_value); oss << "=---"; }
            else if (arg.has_default_value) oss << arg.default_value.toS();
        }
        oss << ")";
        if (metadata.out_args.size == 0) return oss.str();

        oss << "->";
        if (metadata.out_args.size > 1) oss "(";
        for (int i = 0; i < metadata.out_args.size; ++i) {
            Function_arg& arg = metadata->out_args[i];
            if (i > 0) oss << ", ";
            if (arg.is_using) { ASSERT(arg.id.size > 0); oss << "using "; }
            if (arg.id.size > 0) { oss << arg.id.toS() << ":"; }
            oss << arg.type.toS();
            if (arg.explicit_uninitialized) { ASSERT(arg.has_default_value && arg.id.size > 0); oss << "=---"; }
            else if (arg.has_default_value) { ASSERT(arg.id.size > 0); oss << arg.default_value.toS(); }
        }
        if (metadata.out_args.size > 1) oss ")";
        return oss.str();
    }

    bool operator==(const Function_type& o) const { return true; } // all function pointers are the same
    bool operator!=(const Function_type& o) const { return !(*this==o); }

};






struct Function_instance {
    static CB_Type type; // Type "Function_pointer" - only here to conform to standard for a CB value. Special cases are needed elsewhere, to use function_type instead for type checking.
    Function_type function_type;







    static CB_Type type;
    void (*v)() = nullptr; // function pointer

    std::string toS() const {
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



*/