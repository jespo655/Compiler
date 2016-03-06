#include "parser.h"
#include "lexer.h"
#include <iostream>
#include <vector>
using namespace std;


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




int lexer_test()
{
    auto tokens = get_tokens_from_file("test.jai");
    for (const Token& t : tokens) {
        cout << "(file " << t.context.file
            << ", line " << t.context.line
            << ", pos " << t.context.position
            << ") token: " << t.token << endl;
    }
    printf("End of file\n");
}










int main()
{
    return lexer_test();
    // return paren_test();
    cerr << "main.cpp: empty main" << endl;
}






