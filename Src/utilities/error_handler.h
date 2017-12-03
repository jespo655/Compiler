#pragma once

#include <string>

struct Token_context;

void log_error(const std::string& msg, const Token_context& context);
void log_warning(const std::string& msg, const Token_context& context);

void add_note(const std::string& msg, const Token_context& context);
void add_note(const std::string& msg);

void exit_if_errors();

void set_logging(bool); // if set to false, no errors will be logged. Default is true.

