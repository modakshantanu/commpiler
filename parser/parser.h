#ifndef PARSER_H
#define PARSER_H

#include "../common.h"

vector<Token> tokens;
int i = 0;

// ((1 + 2) - 3) * 4
// 


bool matchLiteral() {
    int type = tokens[i++].type;
    return type == INT_LIT || type == STR_LIT || type == FLOAT_LIT || type == CHAR_LIT || type == BOOL_LIT;
}

bool matchIdentifier() {
    return tokens[i++].type == IDEN;
}

bool matchLPar() {
    return tokens[i++].type == L_PAR;
}

bool matchRPar() {
    return tokens[i++].type == R_PAR;
}

bool matchToken(int token) {
    return tokens[i++].type == token;
}

bool matchUnaryOp() {
    int type = tokens[i++].type;
    return type == ADD || type == SUB || type == NOT || type == BNOT;
}

bool matchBinaryOp() {
    int type = tokens[i++].type;
    return type == ADD || type == SUB || type == MUL || type == DIV || type == MOD || type == OR || type == AND || type == EQ || type == LT
        || type == GT || type == LEQ || type == GEQ || type == NEQ || type == BOR || type == BAND || type == BNOT || type == BXOR;
}

bool matchType() {
    int type = tokens[i++].type;
    return type == INT || type == STR || type == FLOAT || type == BOOL || type == CHAR;
}

bool matchExpression();

bool matchExpression2() {
    int save = i;
    bool res;


    res = matchLPar() && matchExpression() && matchRPar();
    if (res) return true;
    i = save;

    res = matchUnaryOp() && matchExpression();
    if (res) return true;
    i = save;

    res = matchLiteral();
    if (res) return true;
    i = save;

    res = matchIdentifier();
    if (res) return true;
    i = save;
    return false;
}

bool matchExpression() {
    int save = i;
    bool res;

    res = matchExpression2() && matchBinaryOp() && matchExpression2();
    if (res) {
        while (res) { // Match multiple Binary - Experssion sequence
            int save2 = i;
            res = matchBinaryOp() && matchExpression2();
            if (!res) i = save2;
        }
        return true;
    }
    i = save;



    res = matchExpression2();
    if (res) return true;
    i = save;

    return false;
}

bool matchStatement() {
    int save = i;
    bool res;

    res = matchIdentifier() && matchToken(ASSIGN) && matchExpression() && matchToken(SEMI);
    if (res) return true;
    i = save;

    res = matchType() && matchIdentifier() && matchToken(ASSIGN) && matchExpression() && matchToken(SEMI);
    if (res) return true;
    i = save;

    return false;
}

bool matchProgram() {
    int save = i;
    bool res; 
    

    res = matchStatement();
    if (res && tokens[i].type == END) return true;
    i = save;

    // Other conditions

    return false;
}

void parseTokens(vector<Token>& tok) {
    tokens = tok;
    i = 0;
    bool res = matchProgram();

    if (res) cout<<"Success";
    else cout<<"Failure";
}





#endif