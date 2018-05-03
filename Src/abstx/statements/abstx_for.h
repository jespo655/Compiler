#pragma once

#include "abstx_statement.h"
#include "../abstx_scope.h"
#include "../expressions/value_expression.h"
#include "../../utilities/unique_id.h"

/*
for (n in range) {}
for (n in range, step=s) {}
for (n in range, index=s) {}
for (n in range, reverse) {}
*/

// @TODO: WORK IN PROGRESS

struct For_range {
    virtual void generate_code(std::ostream& target) = 0;
};

struct Int_range : For_range {

};
struct Double_range : For_range {
    bool double_precision = false;
};
struct Seq_range : For_range {
    std::string index_name = "_it";
    int step = 1;
    bool reverse = false;

    void generate_code(std::ostream& target) override {
        // @todo: Seq.size, Seq[] should both be handled by something else

        static auto uid = get_unique_id();
        // Seq->type.generate_type(target);
        // target << " " << index_name << ";" << std::endl;
        // target << "for(size_t _it_" << uid << " = ";
        // if (reverse) target << "Seq.size-1";
        // else target << "0";
        // target << "; " << index_name << " = " << "Seq.v_ptr[_it_" << uid << "], _it_" << uid;
        // if (reverse) target << " >= 0; _it_" << uid << " -= ";
        // else target << " < " << "Seq.size" << "; _it_" << uid << " += ";
        // target << step << ") ";
    }
};

struct For_statement : Statement {

    enum Range_type {
        INT_RANGE,
        FLOAT_RANGE,
        SEQ_RANGE
    };

    std::string index_name = "_it";
    Owned<Value_expression> range; // range or sequence
    bool reverse = false;
    double step = 1;
    Range_type range_type;

    Owned<Abstx_scope> scope;

    std::string toS() const override { return "while(){}"; }

    void debug_print(Debug_os& os, bool recursive=true) const override
    {
        // FIXME: better for::toS()

        os << "For(";
        if (recursive) {
            ASSERT(range != nullptr);
            os << range->toS();
        }
        os << ") ";
        if (recursive) {
            ASSERT(scope != nullptr);
            scope->debug_print(os, recursive);
        }
        else os << std::endl;
    }

    Parsing_status finalize() override {
        if (!is_codegen_ready(range->status)) status = Parsing_status::DEPENDENCIES_NEEDED;
        else if (!is_codegen_ready(scope->status)) status = Parsing_status::DEPENDENCIES_NEEDED;
        else status = Parsing_status::FULLY_RESOLVED;
        return status;
    }

    void generate_code(std::ostream& target) override {
        ASSERT(is_codegen_ready(status));
        target << "for(";
        switch(range_type) {
            case INT_RANGE: break;
            case FLOAT_RANGE: break;
            case SEQ_RANGE: break;
        }

        scope->generate_code(target);
    }

};


/*

// Generates c-code:

for (double n = range.start; n <= range.end; n+=step) {} // reverse = false
for (double n = range.end; n >= range.start; n-=step) {} // reverse = true

for (int index = 0; i <= (range.end-range.start)/step; i++) {} // step != 1
for (int index = 0; i <= (range.end-range.start); i++) {} // step = 1


*/
