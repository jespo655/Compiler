#pragma once

// #include "../utilities/unique_id.h"
#include "../utilities/assert.h"

#include <map>
#include <string>


// FIXME: remove this and use utilities/unique_id instead
// problem: cannot be declared static in a header file - that
// will make separate instances of "id" in different files
int get_unique_id() {
    static int id=0; // -1 is uninitialized
    ASSERT(id >= 0); // if id is negative then the int has looped around. More than INT_MAX unique identifiers should never be needed.
    return id++;
}



struct CB_Type
{
    static CB_Type type; // self reference / CB_Type
    static std::map<int, std::string> typenames; // mapped from uid to name. Only compile time.

    int uid = get_unique_id();

    CB_Type() {}
    CB_Type(const std::string& name) {
        typenames[uid] = name;
    }

    std::string toS() const {
        const std::string& name = typenames[uid];
        if (name == "") return "type("+std::to_string(uid)+")";
        return name;
    }

    bool operator==(const CB_Type& o) const { return uid == o.uid; }
    bool operator!=(const CB_Type& o) const { return !(*this==o); }
};
std::map<int, std::string> CB_Type::typenames{};
CB_Type CB_Type::type = CB_Type("type");
