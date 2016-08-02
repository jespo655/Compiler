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

struct CB_String {
    static CB_Type type;
    uint32_t size = 0; // size in bytes, not in UTF-8 characters
    uint32_t capacity = 0;
    char* v_ptr = nullptr;

    std::string toS() const {
        ASSERT(v_ptr != nullptr);
        return std::string(v_ptr);
    }

    // default constructor
    CB_String() {
        v_ptr = (char*)malloc(capacity+1);
        memset(v_ptr, 0, capacity+1);
    }
    CB_String(const std::string& str) {
        size = str.length();
        capacity = size;
        v_ptr = (char*)malloc(size+1);
        memcpy(v_ptr, str.c_str(), size+1);
    }
    CB_String(const char* cstr) {
        size = strlen(cstr);
        capacity = size;
        v_ptr = (char*)malloc(size+1);
        memcpy(v_ptr, cstr, size+1);
    }
    ~CB_String() {
        free(v_ptr);
        v_ptr = nullptr;
    }

    bool operator==(const CB_String& str) const { return size == str.size && strcmp(v_ptr, str.v_ptr); }
    bool operator!=(const CB_String& str) const { !(*this == str); }

    // copy
    CB_String& operator=(const CB_String& str) {
        if (capacity < str.size)
            reallocate(str.size, false);
        ASSERT(capacity >= str.size);
        memcpy(v_ptr, str.v_ptr, size+1);
        size = str.size;
        return *this;
    }
    CB_String(const CB_String& str) { *this = str; }

    // move
    CB_String& operator=(CB_String&& str) {
        free(v_ptr);
        size = str.size;
        v_ptr = str.v_ptr;
        str.v_ptr = nullptr;
        return *this;
    }
    CB_String(CB_String&& str) { *this = std::move(str); }

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

CB_Type CB_String::type = CB_Type("string");


