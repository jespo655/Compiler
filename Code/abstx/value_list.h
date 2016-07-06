#pragma once

#include "value_expression.h"
#include "type.h"

#include <vector>
#include <sstream>

struct Type_list : Type
{
    std::vector<std::shared_ptr<Type>> types;

    std::string toS() const override {
        std::ostringstream oss;
        oss << "Type_list(";
        bool first = true;
        for (auto& t : types) {
            if (!first) oss << ", ";
            oss << t->toS();
            first = false;
        }
        oss << ")";
        return oss.str();
    }

    std::string toS(void const * value_ptr, int size=0) const
    {
        ASSERT(false, "type_list should not be used this way"); // FIXME: better error msg
        return "error";
    }

    int byte_size() const override {
        int sum = 0;
        for (auto& t : types) {
            sum += t->byte_size();
        }
        return sum;
    }
};



struct Value_list : Value_expression
{
    std::vector<std::shared_ptr<Value_expression>> expressions;

    std::shared_ptr<Type> get_type() override {
        auto t_list = std::shared_ptr<Type_list>(new Type_list());
        for (auto& expr : expressions) {
            t_list->types.push_back(expr->get_type());
        }
        return t_list;
    }

    std::string toS() const override {
        std::ostringstream oss;
        oss << "Value_list(";
        bool first = true;
        for (auto& expr : expressions) {
            if (!first) oss << ", ";
            oss << expr->toS();
            first = false;
        }
        oss << ")";
        return oss.str();
    }
};



