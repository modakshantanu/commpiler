#include <bits/stdc++.h>
#include "common.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

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
    
    
    vector<Token> tokens = tokenize(str);
    tokens = removeWhitespaces(tokens);
    // printTokens(tokens);
    parseTokens(tokens);

    

}