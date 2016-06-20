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
    auto tokens = get_tokens_from_file("test.jai");
//     auto tokens = get_tokens_from_string("\n\
// string with newlines\n\
// \" and stuff\"");
    // auto tokens = get_tokens_from_string("");

    for (const Token& t : tokens) {
        log_error(t.token,t.context);
        cout << "    Token type:";
        switch(t.type) {
            case Token_type::UNKNOWN: cout << "UNKNOWN" << endl; break;
            case Token_type::SYMBOL: cout << "SYMBOL" << endl; break;
            case Token_type::IDENTIFIER: cout << "IDENTIFIER" << endl; break;
            case Token_type::KEYWORD: cout << "KEYWORD" << endl; break;
            case Token_type::INTEGER: cout << "INTEGER" << endl; break;
            case Token_type::FLOAT: cout << "FLOAT" << endl; break;
            case Token_type::STRING: cout << "STRING" << endl; break;
            case Token_type::BOOL: cout << "BOOL" << endl; break;
            case Token_type::EOF: cout << "EOF" << endl; break;
        }
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
        cout << "  ";
    }
}

void print_statement(const Dynamic_statement* ds, int indent_level);
void print_evaluated_variable(const Evaluated_variable* p, int indent_level);
void print_evaluated_value(const Evaluated_value* p, int indent_level);
void print_identifier(const Identifier* p, int indent_level);


void print_dynamic_scope(const Dynamic_scope* ds, int indent_level)
{
    indent(indent_level); cout << "Dynamic scope:" << endl;
    if (!ds->identifiers.empty()) {
        indent(indent_level+1); cout << "Identifiers: " << ds->identifiers.size() << endl;
        // for (auto& id : ds->identifiers) {
        //     indent(indent_level+2); cout << id->identifier_token->token << ", declared here: " << id->identifier_token->context << endl;
        // }
    }
    if (!ds->imported_scopes.empty()) {
        indent(indent_level+1); cout << "Has " << ds->imported_scopes.size() << " imported scopes." << endl;
    }
    if (!ds->statements.empty()) {
        indent(indent_level+1); cout << "Dynamic statements:" << endl;
        for (auto& s : ds->statements) {
            print_statement(s.get(), indent_level+2);
        }
    }
}

void print_static_scope(const Static_scope* ss, int indent_level)
{
    indent(indent_level); cout << "Static scope:" << endl;
    if (!ss->identifiers.empty()) {
        indent(indent_level+1); cout << "Identifiers: " << ss->identifiers.size() << endl;
        // for (auto& id : ss->identifiers) {
        //     indent(indent_level+2); cout << id->identifier_token->token << ", declared here: " << id->identifier_token->context << endl;
        // }
    }
    if (!ss->imported_scopes.empty()) {
        indent(indent_level+1); cout << "Has " << ss->imported_scopes.size() << " imported scopes." << endl;
    }
    if (!ss->statements.empty()) {
        indent(indent_level+1); cout << "statements:" << endl;
        for (auto& s : ss->statements) {
            print_statement(s.get(), indent_level+2);
        }
    }
}

void print_scope(const Scope* s, int indent_level)
{
    if (const Dynamic_scope* p = dynamic_cast<const Dynamic_scope*>(s)) print_dynamic_scope(p,indent_level);
    else if (const Static_scope* p = dynamic_cast<const Static_scope*>(s)) print_static_scope(p,indent_level);
    else {
        indent(indent_level); cout << "Unknown scope" << endl;
    }
}


void print_range(const Range* p, int indent_level)
{
    indent(indent_level); cout << "Range: from start to end" << endl;
    print_evaluated_value(p->start.get(),indent_level+1);
    print_evaluated_value(p->end.get(),indent_level+1);
}


void print_if(const If_clause* p, int indent_level)
{
    indent(indent_level); cout << "if statement:" << endl;
    indent(indent_level+1); cout << "condition:" << endl;
    print_evaluated_value(p->condition.get(),indent_level+2);
    if (p->if_true != nullptr) {
        indent(indent_level+1); cout << "if_true body:" << endl;
        print_dynamic_scope(p->if_true.get(),indent_level+2);
    }
    if (!(p->elsifs.empty())) {
        for (auto& elsif : p->elsifs) {
            indent(indent_level+1); cout << "elsif with condition:" << endl;
            print_evaluated_value(elsif->condition.get(),indent_level+2);
            indent(indent_level+1); cout << "elsif body:" << endl;
            print_dynamic_scope(elsif->body.get(),indent_level+2);
        }
    }
    if (p->if_false != nullptr) {
        indent(indent_level+1); cout << "if_false body:" << endl;
        print_dynamic_scope(p->if_false.get(),indent_level+2);
    }
    if (p->then_body != nullptr) {
        indent(indent_level+1); cout << "then body:" << endl;
        print_dynamic_scope(p->then_body.get(),indent_level+2);
    }
}

void print_while(const While_clause* p, int indent_level)
{
    indent(indent_level); cout << "while loop:" << endl;
    indent(indent_level+1); cout << "condition:" << endl;
    print_evaluated_value(p->condition.get(),indent_level+2);
    if (p->loop != nullptr) {
        indent(indent_level+1); cout << "loop body:" << endl;
        print_dynamic_scope(p->loop.get(),indent_level+2);
    }
}




void print_for(const For_clause* p, int indent_level) { indent(indent_level); cout << "for (TODO)" << endl; }
void print_using(const Using_statement* p, int indent_level) { indent(indent_level); cout << "using (TODO)" << endl; }
void print_typed_id(const Typed_identifier* p, int indent_level) { indent(indent_level); cout << "typed id (TODO)" << endl; }
void print_value_list(const Value_list* p, int indent_level) { indent(indent_level); cout << "value list (TODO)" << endl; }
void print_function_call(const Function_call* p, int indent_level) { indent(indent_level); cout << "function call (TODO)" << endl; }
void print_getter(const Getter* p, int indent_level) { indent(indent_level); cout << "getter (TODO)" << endl; }
void print_cast(const Cast* p, int indent_level) { indent(indent_level); cout << "cast (TODO)" << endl; }
void print_array_lookup(const Array_lookup* p, int indent_level) { indent(indent_level); cout << "array lookup (TODO)" << endl; }


void print_infix_op(const Infix_op* p, int indent_level)
{
    indent(indent_level); cout << "infix op with operator " << p->op_token->token << endl;
    indent(indent_level+1); cout << "lhs:" << endl;
    print_evaluated_value(p->lhs.get(),indent_level+2);
    indent(indent_level+1); cout << "rhs:" << endl;
    print_evaluated_value(p->rhs.get(),indent_level+2);
}

void print_return_statement(const Return_statement* p, int indent_level)
{
    indent(indent_level); cout << "return statement";
    if (p->return_values.empty()) {
        cout << " returning void" << endl;
    } else {
        cout << " returning these values: " << endl;
        for (auto& evp : p->return_values) {
            print_evaluated_value(evp.second.get(),indent_level+1);
        }
    }
}

void print_function(const Function* p, int indent_level)
{
    indent(indent_level); cout << "Function of type " << p->get_type()->get_type_id() << endl;
    // if (!p->in_parameter_name_tokens.empty()) {
    //     indent(indent_level+1); cout << "in_parameter names:" << endl;
    //     for (Token const* t : p->in_parameter_name_tokens) {
    //         indent(indent_level+2); cout << t->token << endl;
    //     }
    // }
    // if (!p->out_parameter_name_tokens.empty()) {
    //     indent(indent_level+1); cout << "out_parameter names:" << endl;
    //     for (Token const* t : p->out_parameter_name_tokens) {
    //         indent(indent_level+2); cout << t->token << endl;
    //     }
    // }
    indent(indent_level+1); cout << "body:" << endl;
    print_dynamic_scope(p->body.get(),indent_level+2);
}






void print_type_info(const Type_info* p, int indent_level)
{
    indent(indent_level); cout << p->get_type_id() << endl;
}


void print_literal(const Literal* p, int indent_level)
{
    indent(indent_level); cout << "Literal: " << p->literal_token->token << endl;
}


void print_declaration(const Declaration* p, int indent_level)
{
    indent(indent_level); cout << "Declaration:" << endl;
    if (!p->lhs.empty()) {
        indent(indent_level+1); cout << "Declared identifiers:" << endl;
        for (auto& v : p->lhs) {
            indent(indent_level+2);
            if (v.size() > 1) {
                cout << "Lhs group: " << v[0]->identifier_token->token;
                for (int i = 1; i < v.size(); ++i) cout << ", " << v[i]->identifier_token->token;
            } else {
                cout << v[0]->identifier_token->token;
            }
            if (v[0]->type != nullptr) cout << " : " << v[0]->type->get_type_id();
            cout << endl;
        }
    }
    if (!p->rhs.empty()) {
        indent(indent_level+1); cout << "Values:" << endl;
        for (auto& value : p->rhs) {
            print_evaluated_value(value.get(),indent_level+2);
        }
    }
}


void print_assignment(const Assignment* p, int indent_level)
{
    indent(indent_level); cout << "Assignment with operator " << p->assignment_op_token->token << endl;
    if (!p->lhs.empty()) {
        indent(indent_level+1); cout << "Assigned identifiers:" << endl;
        for (auto& v : p->lhs) {
            print_evaluated_variable(v.get(),indent_level+2);
        }
    }
    if (!p->rhs.empty()) {
        indent(indent_level+1); cout << "Values:" << endl;
        for (auto& value : p->rhs) {
            print_evaluated_value(value.get(),indent_level+2);
        }
    }
}


void print_identifier(const Identifier* p, int indent_level)
{
    indent(indent_level); cout << p->identifier_token->token << endl;
}


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
    else if (const Literal* p = dynamic_cast<const Literal*>(val)) print_literal(p,indent_level);
    else if (const Value_list* p = dynamic_cast<const Value_list*>(val)) print_value_list(p,indent_level);
    else if (const Infix_op* p = dynamic_cast<const Infix_op*>(val)) print_infix_op(p,indent_level);
    else if (const Scope* p = dynamic_cast<const Scope*>(val)) print_scope(p,indent_level);
    else if (const Type_info* p = dynamic_cast<const Type_info*>(val)) print_type_info(p,indent_level);
    else if (const Function* p = dynamic_cast<const Function*>(val)) print_function(p,indent_level);
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
    else if (const Return_statement* p = dynamic_cast<const Return_statement*>(ds)) print_return_statement(p,indent_level);
    else {
        indent(indent_level); cout << "Unknown statement" << endl;
    }
}





