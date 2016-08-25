#pragma once

#include "../utilities/assert.h"

#include <map>
#include <string>

struct CB_Any; // used for default values

struct CB_Type
{
    static CB_Type type; // self reference / CB_Type
    static std::map<int, std::string> typenames; // mapped from uid to name. Only compile time.
    static std::map<int, CB_Any> default_values; // mapped from uid to value. Only compile time.

    int uid = get_unique_type_id();

    CB_Type() {}
    CB_Type(const std::string& name) { typenames[uid] = name; }
    template<typename T, typename Type=CB_Type*, Type=&T::type>
    CB_Type(const std::string& name, T&& default_value);
    virtual ~CB_Type() {}

    virtual std::string toS() const {
        const std::string& name = typenames[uid];
        if (name == "") return "anonymous type("+std::to_string(uid)+")";
        return name; // +"("+std::to_string(uid)+")";
    }

    CB_Any default_value() const;

    bool operator==(const CB_Type& o) const { return uid == o.uid; }
    bool operator!=(const CB_Type& o) const { return !(*this==o); }

    static int get_unique_type_id() {
        static int id=0; // -1 is uninitialized
        ASSERT(id >= 0); // if id is negative then the int has looped around. More than INT_MAX unique identifiers should never be needed.
        return id++;
    }

};
std::map<int, std::string> CB_Type::typenames{};
// CB_Type CB_Type::type = CB_Type("type"); // no default value



#include "any.h"
std::map<int, CB_Any> CB_Type::default_values{};
CB_Type CB_Type::type = CB_Type("type", CB_Type("void")); // if we want a default value, this line has to be after the default_values initialization line

template<typename T, typename Type, Type>
CB_Type::CB_Type(const std::string& name, T&& default_value)
{
    typenames[uid] = name;
    default_values[uid] = std::move(CB_Any(default_value));
}

CB_Any CB_Type::default_value() const {
    CB_Any& any = default_values[uid];
    ASSERT(any.v_type == *this, "Type '"+toS()+"' has no default value!");
    return default_values[uid];
}
