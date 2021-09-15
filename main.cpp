#include <bits/stdc++.h>
#include "common.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/semantic.h"
#include "codegen/codegen.h"

using namespace std;

int main(int argc, char* argv[]) {

    

    if (argc <= 1) {
        printf("Missing input file\n");
        return 0;
    }


    string input = argv[1];
    ifstream file(input);

    if (!file) {
        printf("File not found");
        return 0;
    }

    stringstream buf;
    buf << file.rdbuf();
    string str = buf.str();

    // Special functions
    str += "\n\
    void write(int data, int addr) {;}\n\
    int read(int addr) {;}\n\
    ";
    
    
    vector<Token> tokens = tokenize(str);
    tokens = removeWhitespaces(tokens);
    // printTokens(tokens);
    Node* AST = parseTokens(tokens);

    checkSemantics(AST);

    printTree(AST , 0);
    generate(AST);

    

}