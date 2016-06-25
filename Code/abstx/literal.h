#pragma once

#include "evaluated_value.h"
#include "../compile_time/value.h"

/*
Abstx_node wrapper for a Value

For compile time execution:
All subclasses should implement a get_value() function
which returns a value of the corresponding type
*/
struct Literal : Evaluated_value {
    Value value;
    Literal() {}
    Literal(const Value& value) : value{value} {}
    Literal(Value&& value) : value{std::move(value)} {}
    bool initialized() { return value.is_allocated(); }


    template<typename T>
    void get_value(T& dst) {
        ASSERT(sizeof(T) == value.get_type()->byte_size());
        T const * value_ptr = (T const *)value.get_value();
        dst = *value_ptr;
    }

    std::shared_ptr<Type> get_type() override { return value.get_type(); };

    std::string toS() const override {
        auto type = value.get_type();
        return type->toS(value.get_value(), type->byte_size());
    }
};

