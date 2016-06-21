
#include "compile_time.h"
#include "../abstx/workspace.h"
#include "../abstx/evaluated_value.h"
#include "../abstx/function.h"

const std::shared_ptr<Workspace> compiled_workspace{new Workspace()};



void set_entry_point(std::shared_ptr<Function> entry_point, std::vector<std::shared_ptr<Evaluated_value>> arguments) {
    ASSERT(entry_point->fully_resolved);
    // FIXME: type check entry_point and arguments
    compiled_workspace->entry_point = entry_point;
    compiled_workspace->arguments = arguments;
}




std::shared_ptr<Function> get_compile_time_function(std::string id)
{
    return nullptr;
}

void execute_compile_time_function(std::string id, std::vector<std::shared_ptr<Evaluated_value>> arguments)
{

}
