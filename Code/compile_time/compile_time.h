#pragma once

#include <string>
#include <memory>
#include <vector>



struct Function;
struct Evaluated_value;

std::shared_ptr<Function> get_compile_time_function(std::string id);

void execute_compile_time_function(std::string id, std::vector<std::shared_ptr<Evaluated_value>> arguments);


