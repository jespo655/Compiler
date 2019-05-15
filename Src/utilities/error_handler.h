#pragma once

#include <string>

struct Token_context;

void log_error(const std::string& msg, const Token_context& context);
void log_warning(const std::string& msg, const Token_context& context);

void add_note(const std::string& msg, const Token_context& context);
void add_note(const std::string& msg);

void exit_if_errors();
int get_error_count();
void reset_errors();

void set_logging(bool on); // if set to false, no error messages will be logged (they are still counted). Default is true.

