#include "error_handler.h"
#include <iostream>
#include <cstdlib>

int err_count = 0;
int err_max = 100;
bool should_log = true;


void set_logging(bool b)
{
    should_log = b;
}


void exit_if_errors()
{
    if (err_count > 0) {
        std::cerr << "There were errors. Exiting." << std::endl;
        exit(EXIT_FAILURE);
    }
}


void check_for_termination()
{
    if (err_count >= err_max) {
        std::cerr << "Error count reached the maximum of " << err_max << ". Terminating compilation." << std::endl;
        exit(EXIT_FAILURE);
    }
}


void log_error(const std::string& msg, const Token_context& context)
{
    if (!should_log) return;
    std::cerr << std::endl << context.toS() << " Error: " << msg << std::endl;
    err_count++;
}


void log_warning(const std::string& msg, const Token_context& context)
{
    if (!should_log) return;
    std::cerr << std::endl << context.toS() << " Warning: " << msg << std::endl;
}


void add_note(const std::string& msg, const Token_context& context)
{
    if (!should_log) return;
    // std::cerr << context.toS() << " Note: " << msg << std::endl;
    std::cerr << "    Note: " << msg << context.toS() << std::endl;
}


void add_note(const std::string& msg)
{
    if (!should_log) return;
    std::cerr << "    Note: " << msg << std::endl;
}



