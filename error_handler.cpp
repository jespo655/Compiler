#include "error_handler.h"
#include <iostream>
#include <cstdlib>
using namespace std;

int err_count = 0;
int err_max = 100;

void check_for_termination()
{
    if (++err_count >= err_max) {
        cerr << "Error count reached the maximum of " << err_max << ". Terminating compilation." << endl;
        exit(EXIT_FAILURE);
    }
}

void log_error(const string& msg, const Token_context& context)
{
    cerr << "(In " << context.file
        << ", line " << context.line
        << ", position " << context.position << "): "
        << msg << endl;
}

void add_note(const std::string& msg, const Token_context& context)
{
    cerr << "    Note: " << msg
        << "(line " << context.line
        << ", position " << context.position
        << ")" << endl;
}

void add_note(const std::string& msg)
{
    cerr << "    Note: " << msg << endl;
}

