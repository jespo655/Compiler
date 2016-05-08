#include "abstx.h"
#include <sstream>
#include "assert.h"
using namespace std;




string Function_type::get_type_id() const
{
    ostringstream oss{};
    oss << "fn";
    if (!in_parameters.empty()) {
        oss << "(";
        bool first = true;
        for (auto& type : in_parameters) {
            if (!first) oss << ",";
            first = false;
            oss << type->get_type_id();
        }
        oss << ")";
    }
    if (!out_parameters.empty()) {
        oss << "->";
        oss << "(";
        bool first = true;
        for (auto& type : out_parameters) {
            if (!first) oss << ",";
            first = false;
            oss << type->get_type_id();
        }
        oss << ")";
    }
    return oss.str();
}


string Struct_type::get_type_id() const
{
    ostringstream oss{};
    oss << "struct{";
    bool first = true;
    for (auto& member : members) {
        ASSERT(member->identifier_token != nullptr);
        ASSERT(member->type != nullptr);
        if (!first) oss << ",";
        first = false;
        oss << member->identifier_token->token << ":" << member->type->get_type_id();
    }
    oss << "}";
    return oss.str();
}



string Type_list::get_type_id() const
{
    ostringstream oss{};
    oss << "type_list{";
    bool first = true;
    for (auto& t : types) {
        if (!first) oss << ",";
        first = false;
        oss << t->get_type_id();
    }
    oss << "}";
    return oss.str();
}


string Infix_op::get_mangled_op() const
{
    ostringstream oss;
    oss << op_token->token << "(" << lhs->get_type()->get_type_id() << "," << rhs->get_type()->get_type_id() << ")";
    return oss.str();
}




// All get_type() implementations, and other functions that handle types, are located in type_checker.cpp


