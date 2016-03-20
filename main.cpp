#include "parser.h"
#include "lexer.h"
#include "error_handler.h"
#include <iostream>
#include <vector>
using namespace std;


int paren_test()
{
    const vector<Token> tokens = get_tokens_from_file("test.jai");
    for (const Token& t : tokens) {
        if (t.token == "(") {
            const Token* end = read_paren(&t);
            if (end != nullptr && end->token == ")")
                printf("Found paren from (%d:%d) to (%d:%d)\n", t.context.line, t.context.position, end->context.line, end->context.position);
        }
        if (t.token == "[") {
            const Token* end = read_bracket(&t);
            if (end != nullptr && end->token == "]")
                printf("Found bracket from (%d:%d) to (%d:%d)\n", t.context.line, t.context.position, end->context.line, end->context.position);
        }
        if (t.token == "{") {
            const Token* end = read_brace(&t);
            if (end != nullptr && end->token == "}")
                printf("Found brace from (%d:%d) to (%d:%d)\n", t.context.line, t.context.position, end->context.line, end->context.position);
        }
    }
    printf("End of file\n");
}




int lexer_test()
{
    // auto tokens = get_tokens_from_file("test.jai");
    auto tokens = get_tokens_from_string("\n\
string with newlines\n\
\" and stuff\"");
    // auto tokens = get_tokens_from_string("");

    for (const Token& t : tokens) {
        log_error(t.token,t.context);
        // cout << "(file " << t.context.file
        //     << ", line " << t.context.line
        //     << ", pos " << t.context.position
        //     << ") token: " << t.token << endl;
    }
}






void print_scope(const Scope* s, int indent_level = 0);

int parser_test()
{
    Static_scope scope = parse_file("test.jai");
    exit_if_errors();
    cout << "printing syntax tree: " << endl;
    print_scope(&scope);
}






int main()
{
    return parser_test();
    // return lexer_test();
    // return paren_test();
    cerr << "main.cpp: empty main" << endl;
}

















































void indent(int indent_level)
{
    for (int i = 0; i < indent_level; ++i) {
        cout << "    ";
    }
}

void print_statement(const Dynamic_statement* ds, int indent_level);
void print_evaluated_variable(const Evaluated_variable* p, int indent_level);
void print_evaluated_value(const Evaluated_value* p, int indent_level);
void print_identifier(const Identifier* p, int indent_level);


void print_dynamic_scope(const Dynamic_scope* ds, int indent_level)
{
    if (!ds->statements.empty()) {
        indent(indent_level); cout << "Dynamic statements:" << endl;
        for (auto& s : ds->statements) {
            print_statement(s.get(), indent_level+1);
        }
    }
}

void print_static_scope(const Static_scope* ss, int indent_level)
{
    if (!ss->statements.empty()) {
        indent(indent_level); cout << "Static statements:" << endl;
        for (auto& s : ss->statements) {
            print_statement(s.get(), indent_level+1);
        }
    }
}

void print_scope(const Scope* s, int indent_level)
{
    indent(indent_level); cout << "Scope:" << endl;
    if (!s->identifiers.empty()) {
        indent(indent_level); cout << "Identifiers:" << endl;
        for (auto& id : s->identifiers) {
            print_identifier(id.get(), indent_level+1);
        }
    }
    if (!s->imported_scopes.empty()) {
        indent(indent_level); cout << "Has " << s->imported_scopes.size() << " imported scopes." << endl;
    }
    if (const Dynamic_scope* p = dynamic_cast<const Dynamic_scope*>(s)) print_dynamic_scope(p,indent_level);
    else if (const Static_scope* p = dynamic_cast<const Static_scope*>(s)) print_static_scope(p,indent_level);
}


void print_list(const List* p, int indent_level)
{
    indent(indent_level); cout << "list" << endl;
}
void print_literal_range(const Literal_range* p, int indent_level)
{
    indent(indent_level); cout << "literal range" << endl;
}
void print_evaluated_range(const Evaluated_range* p, int indent_level)
{
    indent(indent_level); cout << "evaluated range" << endl;
}

void print_range(const Range* r, int indent_level)
{
    if (const List* p = dynamic_cast<const List*>(r)) print_list(p,indent_level);
    else if (const Literal_range* p = dynamic_cast<const Literal_range*>(r)) print_literal_range(p,indent_level);
    else if (const Evaluated_range* p = dynamic_cast<const Evaluated_range*>(r)) print_evaluated_range(p,indent_level);
    else {
        indent(indent_level); cout << "Unknown range" << endl;
    }
}


void print_if(const If_clause* p, int indent_level) { indent(indent_level); cout << "TODO" << endl; }
void print_while(const While_clause* p, int indent_level) { indent(indent_level); cout << "TODO" << endl; }
void print_for(const For_clause* p, int indent_level) { indent(indent_level); cout << "TODO" << endl; }
void print_using(const Using_statement* p, int indent_level) { indent(indent_level); cout << "TODO" << endl; }
void print_declaration(const Declaration* p, int indent_level) { indent(indent_level); cout << "TODO" << endl; }
void print_assignment(const Assignment* p, int indent_level) { indent(indent_level); cout << "TODO" << endl; }
void print_typed_id(const Typed_identifier* p, int indent_level) { indent(indent_level); cout << "TODO" << endl; }
void print_infix_op(const Infix_op* p, int indent_level) { indent(indent_level); cout << "TODO" << endl; }
void print_value_list(const Value_list* p, int indent_level) { indent(indent_level); cout << "TODO" << endl; }
void print_identifier(const Identifier* p, int indent_level) { indent(indent_level); cout << "TODO" << endl; }
void print_function_call(const Function_call* p, int indent_level) { indent(indent_level); cout << "TODO" << endl; }
void print_getter(const Getter* p, int indent_level) { indent(indent_level); cout << "TODO" << endl; }
void print_cast(const Cast* p, int indent_level) { indent(indent_level); cout << "TODO" << endl; }
void print_array_lookup(const Array_lookup* p, int indent_level) { indent(indent_level); cout << "TODO" << endl; }


void print_evaluated_variable(const Evaluated_variable* var, int indent_level)
{
    if (const Identifier* p = dynamic_cast<const Identifier*>(var)) print_identifier(p,indent_level);
    else if (const Function_call* p = dynamic_cast<const Function_call*>(var)) print_function_call(p,indent_level);
    else if (const Getter* p = dynamic_cast<const Getter*>(var)) print_getter(p,indent_level);
    else if (const Cast* p = dynamic_cast<const Cast*>(var)) print_cast(p,indent_level);
    else if (const Array_lookup* p = dynamic_cast<const Array_lookup*>(var)) print_array_lookup(p,indent_level);
    else {
        indent(indent_level); cout << "Unknown evaluated variable" << endl;
    }
}

void print_evaluated_value(const Evaluated_value* val, int indent_level)
{
    if (const Evaluated_variable* p = dynamic_cast<const Evaluated_variable*>(val)) print_evaluated_variable(p,indent_level);
    else if (const Value_list* p = dynamic_cast<const Value_list*>(val)) print_value_list(p,indent_level);
    else if (const Infix_op* p = dynamic_cast<const Infix_op*>(val)) print_infix_op(p,indent_level);
    else if (const Scope* p = dynamic_cast<const Scope*>(val)) print_scope(p,indent_level);
    else {
        indent(indent_level); cout << "Unknown evaluated value" << endl;
    }
}




void print_statement(const Dynamic_statement* ds, int indent_level)
{
    if (const If_clause* p = dynamic_cast<const If_clause*>(ds)) print_if(p,indent_level);
    else if (const While_clause* p = dynamic_cast<const While_clause*>(ds)) print_while(p,indent_level);
    else if (const For_clause* p = dynamic_cast<const For_clause*>(ds)) print_for(p,indent_level);
    else if (const Using_statement* p = dynamic_cast<const Using_statement*>(ds)) print_using(p,indent_level);
    else if (const Declaration* p = dynamic_cast<const Declaration*>(ds)) print_declaration(p,indent_level);
    else if (const Assignment* p = dynamic_cast<const Assignment*>(ds)) print_assignment(p,indent_level);
    else if (const Function_call* p = dynamic_cast<const Function_call*>(ds)) print_function_call(p,indent_level);
    else if (const Scope* p = dynamic_cast<const Scope*>(ds)) print_scope(p,indent_level);
    else if (const Range* p = dynamic_cast<const Range*>(ds)) print_range(p,indent_level);
    else {
        indent(indent_level); cout << "Unknown statement" << endl;
    }
}





