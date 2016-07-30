

// inn/out template
// get_fn<Arg<CB_i8>, typename T>()
// template<typename T> get_return_type



// template<typename... In_generic_types, typename... Out_generic_types, typename...In_types>
// Args<In_generic_types...>, Args<Out_generic_types...>





struct Function_metadata {
    CB_Dynamic_seq<CB_String> in_ids;
    CB_Dynamic_seq<CB_bool> in_initialized;

    CB_Dynamic_seq<CB_String> out_ids;
    CB_Dynamic_seq<CB_bool> out_initialized;
};


// Helper struct for packing arguments into functions
template<typename...>
struct Args {};

// Empty super class to be able to store the functions in the same list
struct CB_Function_class {
    virtual ~CB_Function_class() {}
};

// Empty default implementation
template <typename In, typename Out>
struct CB_Function : CB_Function_class {};

// Specialization using structs
template <typename... In_types, typename... Out_types> // All types are CB types
struct CB_Function<Args<In_types...>, Args<Out_types...> > : CB_Function_class
{
    static CB_Type type;
    typedef Args<Out_types...> Return_type;
    CB_owning_pointer<Function_metadata> metadata;

    // CB_sharing_pointer<void (*)(In_types... ins, Out_types&... outs)> v = nullptr; // behaves very strangely with function pointers for some reason...
    void (*v)(In_types... ins, Out_types&... outs) = nullptr; // function pointer
    void operator()(In_types... ins, Out_types&... outs)
    {
        ASSERT(v != nullptr);
        (*v)(ins..., outs...);
    }

    CB_Function() {}
    ~CB_Function() { v = nullptr; }

    CB_Function& operator=(const CB_Function& fn) { v = fn.v; return *this; }
    CB_Function(const CB_Function& fn) { *this = fn; }

    CB_Function& operator=(void (*fn)(In_types... ins, Out_types&... outs)) { v = fn; return *this; }
    CB_Function(void (*fn)(In_types... ins, Out_types&... outs)) { *this = fn; }

    CB_Function& operator=(const nullptr_t& fn) { ASSERT(fn == nullptr); v = fn; return *this; }
    CB_Function(const nullptr_t& fn) { *this = fn; }
};

template <typename... In_types, typename... Out_types>
CB_Type CB_Function<Args<In_types...>, Args<Out_types...>>::type = CB_Type();



template<int i>
struct Generic_arg
{
    constexpr static int id = i;
    bool deciding = false;
};

struct Type_match_class {};
template<int i, typename T>
struct Type_match : Type_match_class {};


// Empty default implementation
template <typename In, typename Out>
struct CB_Generic_function : CB_Function_class {};

// Specialization using structs
template <typename... In_generic_types, typename... Out_generic_types> // Types can be CB types or Generic_arg<i>, where each i means one type
struct CB_Generic_function<Args<In_generic_types...>, Args<Out_generic_types...>> : CB_Function_class
{
    static CB_Type type;
    typedef Args<Out_types...> Return_type;
    CB_owning_pointer<Function_metadata> metadata;
    CB_Dynamic_seq<CB_owning_pointer<CB_Function_class>> constructed_fns;
    // Abstx_Generic_fn abstx;

    template<typename In, typename Out>
    CB_sharing_pointer<CB_Function<In, Out>> get_specialization()
    {
        return specialization_generator<In, Out>()(constructed_fns);
    }

private:
    // following the same pattern as before
    template<typename In, typename Out>
    struct specialization_generator {};

    template<typename... In_types, typename... Out_types>
    struct specialization_generator<Args<In_types...>, Args<Out_types...>>
    {
        typedef CB_Function<Args<In_types...>, Args<Out_types...>> fn_type;
        CB_sharing_pointer<fn_type> operator()(CB_Dynamic_seq<CB_owning_pointer<CB_Function_class>>& fns)
        {
            for (auto& ptr : fns) {
                CB_sharing_pointer<fn_type> fn_ptr = dynamic_pointer_cast<fn_type>(ptr);
                if (fn_ptr != nullptr) return fn_ptr;
            }

            // TODO:
            // go through in_generic_types and match for in_types
            // if they match, no problem
            // if generic type (std::is_same<T, Generic_type<i>>) store the match in a list of Type_match_class
            // also check all matches - if already matched that generic, log error

            // if all types are OK, construct a new function by compiling the old abstx (store a pointer to that generic abstx)
            // fn_type fn = compile<In, Out>(abstx);

            // Maybe do the type checking in the compile function instead - that way all log_error stuff happens in the compilation files

            std::cout << "fn generator nyi" << std::endl;
            return nullptr;
        }
    };

};


// template<typename Lhs_t, typename Rhs_t, typename Out_t>
// typedef CB_Generic_function<Args<Lhs_t, Rhs_t>, Args<Out_t>> CB_Infix_operator_fn;

// template<typename In_t, typename Out_t>
// typedef CB_Generic_function<Args<In_t>, Args<Out_t>> CB_Prefix_operator_fn;

struct Operator_metadata {
    CB_uint priority;
};

struct CB_Operator
{
    static CB_Type type;
    CB_owning_pointer<Operator_metadata> metadata;
    CB_Dynamic_seq<CB_owning_pointer<CB_Function_class>> prefix_fns; // filled in only during compilation
    CB_Dynamic_seq<CB_owning_pointer<CB_Function_class>> infix_fns; // filled in only during compilation

    template<typename In, typename Out>
    CB_sharing_pointer<CB_Function<In, Out>> get_prefix()
    {
        for (auto& fn_ptr : prefix_fns) {
            auto prefix_ptr = dynamic_pointer_cast<CB_Function<In, Out>>;
            if (prefix_ptr != nullptr) return prefix_ptr;

            auto generic_ptr = dynamic_pointer_cast<CB_Generic_function<In, Out>>;
            if (generic_ptr != nullptr) {
                prefix_ptr = generic_ptr->template get_specialization<In, Out>();
            }
            return prefix_ptr; // nullptr if not found
        }
    }

    template<typename In, typename Out>
    CB_sharing_pointer<CB_Function<In, Out>> get_infix()
    {
        for (auto& fn_ptr : infix_fns) {
            auto infix_ptr = dynamic_pointer_cast<CB_Function<In, Out>>;
            if (infix_ptr != nullptr) return infix_ptr;

            auto generic_ptr = dynamic_pointer_cast<CB_Generic_function<In, Out>>;
            if (generic_ptr != nullptr) {
                infix_ptr = generic_ptr->template get_specialization<In, Out>();
            }
            return infix_ptr; // nullptr if not found
        }
    }

};






template<int i, bool b>
struct Generic_type {};

template<int i>
struct Generic_type<i, true>
{
    static CB_Type type;
    constexpr static bool deciding = true;
    constexpr static int index = i;
};
template<int i>
struct Generic_type<i, false>
{
    static CB_Type type;
    constexpr static bool deciding = false;
    constexpr static int index = i;
};
template<int i>
CB_Type Generic_type<i, true>::type = CB_Type();
template<int i>
CB_Type Generic_type<i, false>::type = Generic_type<i, true>::type;

