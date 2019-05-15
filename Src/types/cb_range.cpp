#include "cb_range.h"

void CB_Range::generate_typedef(std::ostream& os) const {
    os << "typedef struct { ";
    CB_i64::type->generate_type(os);
    os << " r_start; ";
    CB_i64::type->generate_type(os);
    os << " r_end; } ";
    generate_type(os);
    os << ";" << std::endl;
}
void CB_Range::generate_literal(std::ostream& os, void const* raw_data, const Token_context& context, uint32_t depth) const {
    ASSERT(raw_data);
    os << "(";
    generate_type(os);
    os << "){";
    int64_t const* raw_it = (int64_t const*)raw_data;
    CB_i64::type->generate_literal(os, raw_it, context, depth+1);
    os << ", ";
    CB_i64::type->generate_literal(os, raw_it+1, context, depth+1);
    os << "}";
}
void CB_Range::generate_for(std::ostream& os, const std::string& id, const std::string& it_name, uint64_t step, bool reverse, bool protected_scope) const {
    os << "for (";
    CB_i64::type->generate_type(os);
    os << " " << it_name << " = " << id << (reverse?".r_end":".r_start") << "; ";
    os << it_name << (reverse?" >= ":" <= ") << id << (reverse?".r_start":".r_end") << "; ";
    os << it_name << (reverse?" -= ":" += ") << step << ")";
}



void CB_Float_range::generate_typedef(std::ostream& os) const {
    os << "typedef struct { ";
    CB_f64::type->generate_type(os);
    os << " r_start; ";
    CB_f64::type->generate_type(os);
    os << " r_end; } ";
    generate_type(os);
    os << ";" << std::endl;
}
void CB_Float_range::generate_literal(std::ostream& os, void const* raw_data, const Token_context& context, uint32_t depth) const {
    ASSERT(raw_data);
    os << "(";
    generate_type(os);
    os << "){";
    double const* raw_it = (double const*)raw_data;
    CB_f64::type->generate_literal(os, raw_it, context, depth+1);
    os << ", ";
    CB_f64::type->generate_literal(os, raw_it+1, context, depth+1);
    os << "}";
}
void CB_Float_range::generate_for(std::ostream& os, const std::string& id, const std::string& it_name, uint64_t step, bool reverse, bool protected_scope) const {
    os << "for (";
    CB_f64::type->generate_type(os);
    os << " " << it_name << " = " << id << (reverse?".r_end":".r_start") << "; ";
    os << it_name << (reverse?" >= ":" <= ") << id << (reverse?".r_start":".r_end") << "; ";
    os << it_name << (reverse?" -= ":" += ") << step << ")";
}
