#pragma once

#include <string>
#include <memory>
#include <vector>



struct Function;
struct Value_expression;

std::shared_ptr<Function> get_compile_time_function(std::string id);

void execute_compile_time_function(std::string id, std::vector<std::shared_ptr<Value_expression>> arguments);


Value eval(std::shared_ptr<Value_expression> expr);