#ifndef COMMON_H
#define COMMON_H

#include <bits/stdc++.h>
using namespace std;

struct Token {
    int type;
    string str;
    int line;
    int pos;
};

enum {
    INT, FLOAT, CHAR, STR, BOOL, IF, ELSE, FOR, WHILE, DO, PRINT, DRAW, RETURN,
    ADD, SUB, MUL, DIV, MOD,
    OR, AND, NOT, EQ, GT, LT, GEQ, LEQ, NEQ,
    BOR, BAND, BNOT, BXOR,
    ASSIGN,
    CHAR_LIT, STR_LIT, INT_LIT, BOOL_LIT, FLOAT_LIT,
    IDEN, L_PAR, R_PAR, L_BKT, R_BKT, L_BRC, R_BRC, SEMI, COMMA, END, WS, ERR 
};

vector<string> tokenNames =  {
    "INT", "FLOAT", "CHAR", "STR", "BOOL", "IF", "ELSE", "FOR", "WHILE", "DO", "PRINT", "DRAW", "RETURN",
    "ADD", "SUB", "MUL", "DIV", "MOD",
    "OR", "AND", "NOT", "EQ", "GT", "LT", "GEQ", "LEQ", "NEQ",
    "BOR", "BAND", "BNOT", "BXOR",
    "ASSIGN",
    "CHAR_LIT", "STR_LIT", "INT_LIT", "BOOL_LIT", "FLOAT_LIT",
    "IDEN", "L_PAR", "R_PAR", "L_BKT", "R_BKT", "L_BRC", "R_BRC", "SEMI", "COMMA", "END", "WS", "ERR"
};


#endif