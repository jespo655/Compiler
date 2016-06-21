#pragma once


#include "../abstx/identifier.h"
#include "../abstx/function.h"
#include "../abstx/type.h"

#include "../utilities/assert.h"

#include <memory>
#include <vector>



bool verify_types(std::vector<std::shared_ptr<const Type>> lhs, std::vector<std::shared_ptr<const Type>> rhs)
{
    if (lhs.size() != rhs.size()) return false;
    for (int i = 0; i < lhs.size(); i++)  {
        std::shared_ptr<const Type> l_type = lhs[i];
        std::shared_ptr<const Type> r_type = rhs[i];
        if (*l_type != *r_type) return false;
    }
    return true;
}


template<typename T>
std::vector<std::shared_ptr<const Type>> get_types(std::vector<std::shared_ptr<T>> values)
{
    std::vector<std::shared_ptr<const Type>> types;
    for (auto t_ptr : values) {
        if (auto v = std::dynamic_pointer_cast<Evaluated_value>(t_ptr)) {
            types.push_back(v->get_type());
        } else {
            // log error?
            ASSERT(false, "Type checker: Unable to get type from non-Evaluated_value");
        }
    }
}