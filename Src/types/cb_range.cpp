#include "cb_range.h"

namespace Cube {

void CB_Iterable::generate_typedef(std::ostream& os) const override {
    os << "typedef struct { ";
    CB_i64::type->generate_type(os);
    os << " r_start; ";
    CB_i64::type->generate_type(os);
    os << " r_end; } ";
    generate_type(os);
    os << ";" << std::endl;
}
void CB_Iterable::generate_literal(std::ostream& os, void const* raw_data, uint32_t depth = 0) const override {
    ASSERT(raw_data);
    os << "(";
    generate_type(os);
    os << "){";
    int64_t const* raw_it = (int64_t const*)raw_data;
    CB_i64::type->generate_literal(os, raw_it, depth+1);
    os << ", ";
    CB_i64::type->generate_literal(os, raw_it+1, depth+1);
    os << "}";
}
void CB_Iterable::generate_for(std::ostream& os, const std::string& id, const std::string& it_name = "it", uint64_t step = 1, bool reverse = false, bool protected_scope = true) const override {
    os << "for (";
    CB_i64::type->generate_type(os);
    os << " " << it_name << " = " << id << (reverse?".r_end":".r_start") << "; ";
    os << it_name << (reverse?" >= ":" <= ") << id << (reverse?".r_start":".r_end") << ";";
    os << it_name << (reverse?" -= ":" += ") << step << ")";
}



void CB_Float_range::generate_typedef(std::ostream& os) const override {
    os << "typedef struct { ";
    CB_f64::type->generate_type(os);
    os << " r_start; ";
    CB_f64::type->generate_type(os);
    os << " r_end; } ";
    generate_type(os);
    os << ";" << std::endl;
}
void CB_Float_range::generate_literal(std::ostream& os, void const* raw_data, uint32_t depth = 0) const override {
    ASSERT(raw_data);
    os << "(";
    generate_type(os);
    os << "){";
    double const* raw_it = (double const*)raw_data;
    CB_f64::type->generate_literal(os, raw_it, depth+1);
    os << ", ";
    CB_f64::type->generate_literal(os, raw_it+1, depth+1);
    os << "}";
}
void CB_Float_range::generate_for(std::ostream& os, const std::string& id, const std::string& it_name = "it", uint64_t step = 1, bool reverse = false, bool protected_scope = true) const override {
    os << "for (";
    CB_f64::type->generate_type(os);
    os << " " << it_name << " = " << id << (reverse?".r_end":".r_start") << "; ";
    os << it_name << (reverse?" >= ":" <= ") << id << (reverse?".r_start":".r_end") << ";";
    os << it_name << (reverse?" -= ":" += ") << step << ")";
}

}
