#pragma once

#include "abstx_statement.h"
#include "abstx_scope.h"
#include "../expressions/value_expression.h"
#include "../../types/cb_primitives.h"

/*
while (b) {}
*/

struct While_statement : Statement {

    owned<Value_expression> condition;
    owned<CB_Scope> scope;

    std::string toS() const override { return "while(){}"; }

    void debug_print(Debug_os& os, bool recursive=true) const override
    {
        os << "while(";
        if (recursive) {
            ASSERT(condition != nullptr);
            os << condition->toS();
        }
        os << ") ";
        if (recursive) {
            ASSERT(scope != nullptr);
            scope->debug_print(os, recursive);
        }
        else os << std::endl;
    }

    Parsing_status finalize() override {
        if (is_codegen_ready(status)) return status;

        if (!is_codegen_ready(condition->finalize())) {
            status = Parsing_status::DEPENDENCIES_NEEDED;
            return status;
        }
        seq<shared<const CB_Type>> types = condition->get_type();
        if (types.size != 1 || *types[0] != *CB_Bool::type) {
            status = Parsing_status::TYPE_ERROR;
            return status;
        }
        if (!is_codegen_ready(scope->finalize())) {
            status = Parsing_status::DEPENDENCIES_NEEDED;
            return status;
        }
        // we reached the end -> we are done
        status = Parsing_status::FULLY_RESOLVED;
        return status;
    }

    void generate_code(std::ostream& target) override {
        ASSERT(is_codegen_ready(status));
        target << "while (";
        condition->generate_code(target);
        target << ") ";
        scope->generate_code(target);
        status = Parsing_status::CODE_GENERATED;
    };


};


/*

// Generates c-code:

while (b) {}

*/
