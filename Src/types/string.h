#pragma once

#include "type.h"

#include <string>
#include <cstring> // strlen, strcmp

/*
String - Worls like a dynamic array of characters
Actual allocated size is size+1 - the last character is always '\0'

Operating on individual chars is currently not allowed, since it would behave strangely when using utf-8.

Syntax:
a : String = "text";
*/

struct CB_String : CB_Type {
    static CB_Type type;
    static const bool primitive = false;

    CB_Range() { uid = type.uid; }
    std::string toS() const override { return "string"; }

    virtual ostream& generate_typedef(ostream& os) const override {
        // @TODO
        // os << "typedef struct { ";
        // CB_f64::type.generate_type(os);
        // os << " r_start; "
        // CB_f64::type.generate_type(os);
        // " r_end; } ";
        // generate_type(os);
        // return os << ";";
    }
    virtual ostream& generate_literal(ostream& os, void const* raw_data) const override {
        ASSERT(raw_data);
        // @TODO
        // os << "(";
        // generate_type(os);
        // os << "){";
        // double* raw_it = raw_data;
        // CB_f64::type.generate_literal(os, raw_it);
        // os << ", ";
        // CB_f64::type.generate_literal(os, raw_it+1);
        // os << "}";
    }
}











// Below: utilities version of any that can be used in c++

struct _string {
    uint32_t size = 0; // size in bytes, not in UTF-8 characters
    uint32_t capacity = 0;
    char* v_ptr = nullptr;

    std::string toS() const override {
        ASSERT(v_ptr != nullptr);
        return std::string(v_ptr);
    }

    CB_Object* heap_copy() const override { _string* tp = new _string(); *tp = *this; return tp; }

    // default constructor
    _string() {
        v_ptr = (char*)malloc(capacity+1);
        memset(v_ptr, 0, capacity+1);
    }
    _string(const std::string& str) {
        size = str.length();
        capacity = size;
        v_ptr = (char*)malloc(size+1);
        memcpy(v_ptr, str.c_str(), size+1);
    }
    _string(const char* cstr) {
        size = strlen(cstr);
        capacity = size;
        v_ptr = (char*)malloc(size+1);
        memcpy(v_ptr, cstr, size+1);
    }

    // copy
    _string& operator=(const _string& str) {
        if (capacity < str.size)
            reallocate(str.size, false);
        ASSERT(capacity >= str.size);
        ASSERT(v_ptr != nullptr);
        size = str.size;
        memcpy(v_ptr, str.v_ptr, size+1);
        return *this;
    }
    _string(const _string& str) {
        v_ptr = (char*)malloc(capacity+1);
        memset(v_ptr, 0, capacity+1);
        *this = str;
    }

    // move
    _string& operator=(_string&& str) {
        free(v_ptr);
        size = str.size;
        v_ptr = str.v_ptr;
        str.v_ptr = nullptr;
        return *this;
    }
    _string(_string&& str) {
        *this = std::move(str);
    }

    // destructor
    ~_string() {
        free(v_ptr);
        v_ptr = nullptr;
    }

    bool operator==(const _string& str) const { return size == str.size && strcmp(v_ptr, str.v_ptr) == 0; }
    bool operator!=(const _string& str) const { !(*this == str); }
    bool operator<(const _string& str) const { return strcmp(v_ptr, str.v_ptr) < 0; }
    bool operator>=(const _string& str) const { return !(*this < str); }
    bool operator>(const _string& str) const { return str < *this; }
    bool operator<=(const _string& str) const { return !(str < *this); }

    void reallocate(uint32_t capacity, bool move=true) {
        this->capacity = capacity;
        char* new_ptr = (char*)malloc(capacity+1);
        ASSERT(new_ptr != nullptr);
        if (capacity < size) size = capacity;
        if (move) memcpy(new_ptr, v_ptr, size+1);
        free(v_ptr);
        v_ptr = new_ptr;
    }
};

