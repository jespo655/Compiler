#pragma once

#include "../utilities/assert.h"
#include "../abstx/type.h"

#include <memory>
#include <string>
#include <cstring> // memset


struct Value {

    void alloc(std::shared_ptr<Type> t, bool initialize=true) {
        ASSERT(t != nullptr)
        ASSERT(type == nullptr || *type == *t);
        type = t;
        if (!is_allocated()) {
            value_ptr = malloc(type->byte_size());
            if (initialize) {
                memset(value_ptr, 0, type->byte_size());
                initialized = true;
            }
        }
        ASSERT(is_allocated()); // just in case malloc fails
    }

    void free() {
        if (is_allocated()) {
            std::free(value_ptr);
            value_ptr = nullptr;
        }
        ASSERT(!is_allocated());
    }

    void assign(const Value& v) {
        ASSERT(v.is_allocated());
        if (!is_allocated()) alloc(v.type);
        ASSERT(*type == *(v.type));
        memcpy(value_ptr, v.value_ptr, type->byte_size());
        initialized = true;
    }

    template<typename T>
    void assign(std::shared_ptr<Type> type, const T& value) {
        ASSERT(type->byte_size() == sizeof(T));
        alloc(type);
        memcpy(value_ptr, &value, type->byte_size());
        initialized = true;
    }

    void move_from(Value&& v) {
        ASSERT(type == nullptr || v.type == nullptr || *type == *v.type);
        type = std::move(v.type);
        value_ptr = v.value_ptr;
        v.type = nullptr;
        v.value_ptr = nullptr;
    }

    std::shared_ptr<Type> get_type() const { return type; }
    void const * get_value() const { return value_ptr; }

    Value() {}
    ~Value() { free(); }
    Value(const Value& v) { assign(v); }
    Value(Value&& v) { move_from(std::move(v)); }
    Value& operator=(const Value& v) { free(); assign(v); return *this; }
    Value& operator=(Value&& v) { free(); move_from(std::move(v)); return *this; }
    Value(std::shared_ptr<Type> type) { alloc(type); }
    template<typename T> Value(std::shared_ptr<Type> type, const T& value) { assign(type, value); }


    bool is_allocated() const {
        return type != nullptr && value_ptr != nullptr;
    }

    bool is_initialized() const {
        return initialized;
    }

    std::string toS() {
        if (type == nullptr) return "value(uninizialized)";
        return "value(" + type->toS() + ")";
    }

private:

    std::shared_ptr<Type> type = nullptr;
    void* value_ptr = nullptr;
    bool initialized = false;

};



static Value get_default_value(std::shared_ptr<Type> type) {
    // if relevant, add special cases here if some types should have different default values.
    // if (auto t = dynamic_pointer_cast<Type_i8>(type)) return Value(type, (int_least8_t)137); // for testing and demonstrating purposes
    return Value(type); // memsets to 0
}