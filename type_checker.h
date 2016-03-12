#ifndef type_checker_h
#define type_checker_h

#include <string>
#include "lexer.h"

/*
struct Type_info
{
    std::string type;
    unsigned int size = 0; // bytes

    Type_info(const std::string& t = "", unsigned int s = 0) : type{t}, size{s} {}
    bool operator<(const Type_info& o) const { return type < o.type; } // for sorting
    bool operator==(const Type_info& o) const { return type == o.type && size == o.size; }
    bool operator!=(const Type_info& o) const { return !(*this == o); }
    virtual ~Type_info() {}
    virtual Type_info& get_casted_type() { return *this; }
};




struct Castable_type : Type_info
{
    Type_info* casted_type = nullptr; // owned by the castable-type

    Castable_type() {}
    Castable_type(const Type_info& t) : Type_info{t} {}
    Castable_type(const Castable_type& o) : Type_info{o} { deep_copy_cast_type_ptr(o); }
    Castable_type(Castable_type&& o) : Type_info{std::move(o)}, casted_type{o.casted_type} { o.casted_type = nullptr; }
    Castable_type& operator=(const Castable_type& o) { type = o.type; size = o.size; delete casted_type; deep_copy_cast_type_ptr(o); }
    Castable_type& operator=(Castable_type&& o) { type = std::move(o.type); size = o.size; casted_type = o.casted_type; o.casted_type = nullptr; }
    Type_info& get_casted_type() { return (casted_type==nullptr)? *this : casted_type->get_casted_type(); }

private:
    void deep_copy_cast_type_ptr(const Castable_type& o);
};





struct Identifier
{
    std::string name;
    Type_info type;
    void* value_ptr; // mallocs and frees with construction/destruction of the object. Don't change this!
    Token_context context; // definiton context

    Identifier(); // no name and undefined type
    Identifier(const std::string& s, const Type_info& t); // malloc value_ptr
    virtual ~Identifier(); // free value_ptr
    bool operator<(const Identifier& o) const { return name < o.name; } // for sorting
    bool operator==(const Identifier& o) const { return name == o.name; }
    Identifier(const Identifier&);
    Identifier(Identifier&&);
    Identifier& operator=(const Identifier&);
    Identifier& operator=(Identifier&&);
};

*/





#endif