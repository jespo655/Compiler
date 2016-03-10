#include "error_handler.h"
#include <iostream>
#include <cstdlib>
using namespace std;

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
        cerr << "There were errors. Exiting." << endl;
        exit(EXIT_FAILURE);
    }
}


void check_for_termination()
{
    if (++err_count >= err_max) {
        cerr << "Error count reached the maximum of " << err_max << ". Terminating compilation." << endl;
        exit(EXIT_FAILURE);
    }
}

void log_error(const string& msg, const Token_context& context)
{
    if (!should_log) return;
    if (!context.file.empty()) {
        cerr << endl << "(In " << context.file << ", ";
    } else {
        cerr << endl << "(At ";
    }
    cerr << "line " << context.line
        << ", position " << context.position << "): "
        << msg << endl;
    err_count++;
}

void add_note(const std::string& msg, const Token_context& context)
{
    if (!should_log) return;
    cerr << "    Note: " << msg
        << "(line " << context.line
        << ", position " << context.position
        << ")" << endl;
}

void add_note(const std::string& msg)
{
    if (!should_log) return;
    cerr << "    Note: " << msg << endl;
}



