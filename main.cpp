#include "parser.h"
#include "lexer.h"
#include "error_handler.h"
#include <iostream>
#include <vector>
using namespace std;

/*
int paren_test()
{
    const vector<Token> tokens = get_tokens_from_file("test.jai");
    for (const Token& t : tokens) {
        if (t.token == "(") {
            const Token* end = read_paren(tokens,&t);
            if (end != nullptr && end->token == ")")
                printf("Found paren from (%d:%d) to (%d:%d)\n", t.context.line, t.context.position, end->context.line, end->context.position);
        }
        if (t.token == "[") {
            const Token* end = read_bracket(tokens,&t);
            if (end != nullptr && end->token == "]")
                printf("Found bracket from (%d:%d) to (%d:%d)\n", t.context.line, t.context.position, end->context.line, end->context.position);
        }
        if (t.token == "{") {
            const Token* end = read_brace(tokens,&t);
            if (end != nullptr && end->token == "}")
                printf("Found brace from (%d:%d) to (%d:%d)\n", t.context.line, t.context.position, end->context.line, end->context.position);
        }
    }
    printf("End of file\n");
}
*/



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






/*
void indent(int indent_level)
{
    for (int i = 0; i < indent_level; ++i) {
        cout << "    ";
    }
}

void print_abs_stx(Abs_syntax* abstx, int indent_level = 0);

int parser_test()
{
    unique_ptr<Scope> scope = parse_file("test.jai");
    exit_if_errors();
    cout << "printing syntax tree: " << endl;
    print_abs_stx(scope.get());
}


*/




int main()
{
    // return parser_test();
    return lexer_test();
    // return paren_test();
    cerr << "main.cpp: empty main" << endl;
}










































/*

ostream& operator << (ostream& os, const Abs_syntax* abstx)
{
    if (abstx == nullptr) os << "nullptr";
    else os << "(" << abstx->start_token->context.line << "," << abstx->start_token->context.position
        << ") - (" << abstx->end_token->context.line << "," << abstx->end_token->context.position
        << ")";
    return os;
}

ostream& operator << (ostream& os, const unique_ptr<Abs_syntax> abstx)
{
    return os << abstx.get();
}

ostream& operator << (ostream& os, const Token* t)
{
    if (t == nullptr) os << "nullptr";
    else os << t->token;
    return os;
}

void print_abs_stx(Abs_syntax* abstx, int indent_level)
{
    if (abstx == nullptr) {
        indent(indent_level); cout << "nullptr" << endl;
    }

    else if (While_clause* p = dynamic_cast<While_clause*>(abstx)) {
        indent(indent_level); cout << "While_clause: " << endl;
        indent(indent_level); cout << "Condition: " << (Abs_syntax*)p->condition.get() << endl;
        indent(indent_level); cout << "Loop: " << (Abs_syntax*)p->loop.get() << endl;
        print_abs_stx(p->loop.get(), indent_level+1);
    }

    else if (For_clause* p = dynamic_cast<For_clause*>(abstx)) {
        indent(indent_level); cout << "For_clause: " << endl;
        indent(indent_level); cout << "Range: " << (Abs_syntax*)p->range.get() << endl;
        print_abs_stx(p->range.get(), indent_level+1);
        indent(indent_level); cout << "Loop: " << (Abs_syntax*)p->loop.get() << endl;
        print_abs_stx(p->loop.get(), indent_level+1);
    }

    else if (Range* p = dynamic_cast<Range*>(abstx)) {
        indent(indent_level); cout << "Range: " << endl;
        if (p->in_token != nullptr) indent(indent_level); cout << "has in_token" << endl;
        if (p->by_token != nullptr) indent(indent_level); cout << "has by_token" << endl;
    }

    else if (If_clause* p = dynamic_cast<If_clause*>(abstx)) {
        indent(indent_level); cout << "If_clause: " << endl;
        indent(indent_level); cout << "Condition: " << (Abs_syntax*)p->condition.get() << endl;
        indent(indent_level); cout << "If_true: " << (Abs_syntax*)p->if_true.get() << endl;
        print_abs_stx(p->if_true.get(), indent_level+1);
        indent(indent_level); cout << "If_false: " << (Abs_syntax*)p->if_false.get() << endl;
        print_abs_stx(p->if_false.get(), indent_level+1);
    }

    else if (Infix_op* p = dynamic_cast<Infix_op*>(abstx)) {
        indent(indent_level); cout << "Infix_op: " << endl;
        indent(indent_level); cout << "Lhs: " << (Abs_syntax*)p->lhs.get() << endl;
        print_abs_stx(p->lhs.get(),indent_level+1);
        indent(indent_level); cout << "Rhs: " << (Abs_syntax*)p->rhs.get() << endl;
        print_abs_stx(p->rhs.get(),indent_level+1);
        indent(indent_level); cout << "Operator: " << p->op_token << endl; // (p->op_token?p->op_token->token:"nullptr")
    }

    else if (Getter* p = dynamic_cast<Getter*>(abstx)) {
        indent(indent_level); cout << "Getter: " << endl;
        indent(indent_level); cout << "Struct id: " << (Abs_syntax*)p->struct_identifier.get() << endl;
        print_abs_stx(p->struct_identifier.get(),indent_level+1);
        indent(indent_level); cout << "Data id: " << p->data_identifier_token << endl;
    }

    else if (Function_call* p = dynamic_cast<Function_call*>(abstx)) {
        indent(indent_level); cout << "Function_call: " << endl;
        indent(indent_level); cout << "Function id: " << (Abs_syntax*)p->function_identifier.get() << endl;
        print_abs_stx(p->function_identifier.get(),indent_level+1);
        indent(indent_level); cout << "Arguments: " << (Abs_syntax*)p->arguments.get() << endl;
        print_abs_stx(p->arguments.get(),indent_level+1);
    }

    else if (Assignment* p = dynamic_cast<Assignment*>(abstx)) {
        indent(indent_level); cout << "Assignment: " << endl;
        indent(indent_level); cout << "Operator: " << p->op_token->token << endl; // (p->op_token?p->op_token->token:"nullptr")
        indent(indent_level); cout << "Lhs: " << (Abs_syntax*)p->lhs.get() << endl; // this is working fine
        print_abs_stx(p->lhs.get(),indent_level+1);
        indent(indent_level); cout << "Rhs: " << (Abs_syntax*)p->rhs.get() << endl; // this is crashing. ?????
        print_abs_stx(p->rhs.get(),indent_level+1);
    }

    else if (Declaration* p = dynamic_cast<Declaration*>(abstx)) {
        indent(indent_level); cout << "Declaration: " << endl;
        for (unique_ptr<Abs_identifier>& id : p->identifiers) {
            indent(indent_level); cout << "Id: " << (Abs_syntax*)id.get() << endl;
            print_abs_stx(id.get(),indent_level+1);
        }
    }

    else if (Rhs* p = dynamic_cast<Rhs*>(abstx)) {
        indent(indent_level); cout << "Rhs: " << endl;
        for (unique_ptr<Abs_identifier>& id : p->identifiers) {
            indent(indent_level); cout << "Id: " << (Abs_syntax*)id.get() << endl;
            print_abs_stx(id.get(),indent_level+1);
        }
    }

    else if (Lhs* p = dynamic_cast<Lhs*>(abstx)) {
        indent(indent_level); cout << "Lhs: " << endl;
        for (unique_ptr<Lhs_part>& id : p->parts) {
            indent(indent_level); cout << "Part: " << (Abs_syntax*)id.get() << endl;
            print_abs_stx(id.get(),indent_level+1);
        }
    }

    else if (Lhs_part* p = dynamic_cast<Lhs_part*>(abstx)) {
        indent(indent_level); cout << "Lhs_part: " << endl;
        for (unique_ptr<Abs_identifier>& id : p->identifiers) {
            indent(indent_level); cout << "Id: " << (Abs_syntax*)id.get() << endl;
            print_abs_stx(id.get(),indent_level+1);
        }
    }

    else if (Scope* p = dynamic_cast<Scope*>(abstx)) {
        indent(indent_level); cout << "Scope: " << endl;
        if (p->capture_group != nullptr) { indent(indent_level); cout << "Has capture group." << endl; }
        if (!p->imported_scopes.empty()) { indent(indent_level); cout << "Has " << p->imported_scopes.size() << " imported scopes." << endl; }
        for (unique_ptr<Abs_identifier>& id : p->identifiers) {
            indent(indent_level); cout << "Id: " << (Abs_syntax*)id.get() << endl;
            print_abs_stx(id.get(),indent_level+1);
        }
        for (unique_ptr<Statement>& id : p->statements) {
            indent(indent_level); cout << "Statement: " << (Abs_syntax*)id.get() << endl;
            print_abs_stx(id.get(),indent_level+1);
        }
    }

    else if (Function_scope* p = dynamic_cast<Function_scope*>(abstx)) {
        indent(indent_level); cout << "Function_scope: " << endl;
        if (p->parent_scope != nullptr) { indent(indent_level); cout << "Has parent scope." << endl; }
        if (p->capture_group != nullptr) { indent(indent_level); cout << "Has capture group." << endl; }
        for (unique_ptr<Statement>& id : p->statements) {
            indent(indent_level); cout << "Statement: " << (Abs_syntax*)id.get() << endl;
            print_abs_stx(id.get(),indent_level+1);
        }
    }

    else {
        indent(indent_level); cout << "Unknown abs.stx: " << abstx << endl;
    }
}


*/