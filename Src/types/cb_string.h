#pragma once

#include "cb_type.h"
#include "../utilities/pointers.h"

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
    static const shared<const CB_Type> type;
    static const bool primitive = false;
    static constexpr char _default_str[] = "";
    static constexpr char const* _default_value = _default_str;

    CB_String() { uid = type->uid; }
    CB_String(const std::string& name, size_t size, void const* default_value) : CB_Type(name, size, default_value) {}
    std::string toS() const override { return "string"; }

    void generate_typedef(ostream& os) const override {
        // for now, just use regular null-terminated char*
        os << "typedef char* ";
        generate_type(os);
        os << ";";
    }
    void generate_literal(ostream& os, void const* raw_data, uint32_t depth = 0) const override {
        ASSERT(raw_data);
        char const* raw_it = *(char const**)raw_data;
        if (!raw_it) os << "NULL";
        os << "\"";
        while (*raw_it)  {
            switch(*raw_it) {
                case '\n': os << "\\n"; break;
                case '\r': os << "\\r"; break;
                case '\"': os << "\\\""; break;
                case '\\': os << "\\\\"; break;
                default: os << *(char*)raw_it;
            }
        }
        os << "\"";
    }

    std::string parse_raw_data(void* raw_data) {
        return std::string((char*)raw_data);
    }

};











// Below: utilities version of any that can be used in c++

struct _string {
    uint32_t size = 0; // size in bytes, not in UTF-8 characters
    uint32_t capacity = 0;
    char* v_ptr = nullptr;

    std::string toS() const {
        ASSERT(v_ptr != nullptr);
        return std::string(v_ptr);
    }

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

