#pragma once

#include "value.h"

#include <string>
#include <memory>
#include <vector>


/*

NOTHING HERE IS IMPLEMENTED
TODO: IMPLEMENT

*/



struct Function;
struct Value_expression;

std::shared_ptr<Function> get_compile_time_function(std::string id);

void execute_compile_time_function(std::string id, std::vector<std::shared_ptr<Value_expression>> arguments);


Value eval(std::shared_ptr<Value_expression> expr);