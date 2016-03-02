#ifndef error_handler_h
#define error_handler_h

#include "lexer.h" // Token_context
#include <string>

#define DEBUG

#include "assert.h"

void log_error(const std::string& msg, const Token_context& context);

void add_note(const std::string& msg, const Token_context& context); // prints out the line and postition after the message
void add_note(const std::string& msg);

#endif