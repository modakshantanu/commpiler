#ifndef COMMON_H
#define COMMON_H

#include <bits/stdc++.h>
using namespace std;



enum {
    // Terminals
    INT, FLOAT, CHAR, STR, BOOL, VOID, IF, ELSE, FOR, WHILE, DO, RETURN,
    ADD, SUB, MUL, DIV, MOD, UADD, USUB, 
    OR, AND, NOT, EQ, GT, LT, GEQ, LEQ, NEQ,
    BOR, BAND, BNOT, BXOR, L_SHIFT, R_SHIFT,
    ASSIGN,
    CHAR_LIT, STR_LIT, INT_LIT, BOOL_LIT, FLOAT_LIT,
    IDEN, L_PAR, R_PAR, L_BKT, R_BKT, L_BRC, R_BRC, SEMI, COMMA, END, WS, ERR,
    // Non-terminals
    PROGRAM, PARAM_LIST, ARG_LIST, STATEMENT, EXPRESSION, EXPRESSION2, LITERAL, UNARY_OP, BINARY_OP, FUNCTION_CALL, INDEXED, DECLARATION,
    BLOCK, FUNCTION_DECL, DECLASSIGN, DECLONLY, IFELSE, EMPTY, NONE

};

vector<string> tokenNames =  {
    // Terminals
    "INT", "FLOAT", "CHAR", "STR", "BOOL", "VOID", "IF", "ELSE", "FOR", "WHILE", "DO", "RETURN",
    "ADD", "SUB", "MUL", "DIV", "MOD", "UADD", "USUB",
    "OR", "AND", "NOT", "EQ", "GT", "LT", "GEQ", "LEQ", "NEQ",
    "BOR", "BAND", "BNOT", "BXOR" , "L_SHIFT" , "R_SHIFT",
    "ASSIGN",
    "CHAR_LIT", "STR_LIT", "INT_LIT", "BOOL_LIT", "FLOAT_LIT",
    "IDEN", "L_PAR", "R_PAR", "L_BKT", "R_BKT", "L_BRC", "R_BRC", "SEMI", "COMMA", "END", "WS", "ERR",

    // Non terminals
    "PROGRAM", "PARAM_LIST","ARG_LIST","STATEMENT", "EX", "EX2", "LITERAL", "UNARY_OP" , "BINARY_OP", "FUNCTION_CALL", "INDEXED", "DECLARATION",
    "BLOCK", "FUNCTION_DECL", "DECLASSIGN", "DECLONLY", "IFELSE" , "EMPTY", ""
};

struct Token {
    int type;
    string str;
    int line;
    int pos;
    int subtype = NONE;
};

struct Node {
    Token token;
    Node* parent;
    vector<Node*> children;
};


#endif