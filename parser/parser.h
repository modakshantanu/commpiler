#ifndef PARSER_H
#define PARSER_H

#include "../common.h"

struct Node {
    Token token;
    Node* parent;
    vector<Node*> children;
};

vector<Node*> temp;

vector<Token> tokens;
int i = 0;
void printTree(Node* cur, int level);

bool matchLiteral(Node* parent) {
    Token t = tokens[i++];
    bool res = t.type == INT_LIT || t.type == STR_LIT || t.type == FLOAT_LIT || t.type == CHAR_LIT || t.type == BOOL_LIT;
    if (res) {
        parent->children.push_back(new Node{t, parent});
    }
    return res;
}

bool matchToken(Node* parent, int token) {
    Token t = tokens[i++];
    bool res = t.type == token;
    if (res) {
        parent->children.push_back(new Node{t, parent});
    }
    return res;
}

bool matchUnaryOp(Node* parent) {
    Token t = tokens[i++];
    bool res = t.type == ADD || t.type == SUB || t.type == NOT || t.type == BNOT;
    if (res) {
        parent->children.push_back(new Node{t, parent});
    }
    return res;
}

bool matchBinaryOp(Node* parent) {

    Token t = tokens[i++];
    bool res = t.type == ADD || t.type == SUB || t.type == MUL || t.type == DIV || t.type == MOD || t.type == OR || t.type == AND || t.type == EQ || t.type == LT
        || t.type == GT || t.type == LEQ || t.type == GEQ || t.type == NEQ || t.type == BOR || t.type == BAND || t.type == BNOT || t.type == BXOR;

    if (res) {
        parent->children.push_back(new Node{t, parent});
    }
    return res;
    
}

bool matchType(Node* parent) {
    Token t = tokens[i++];
    bool res = t.type == INT || t.type == STR || t.type == FLOAT || t.type == BOOL || t.type == CHAR;
    if (res) {
        parent->children.push_back(new Node{t, parent});
    }
    return res;
}

void freeNodes(Node* head) {
    if (head == NULL) return;
    for (auto it: head->children) free(it);
    free(head);
}

bool matchExpression(Node* parent);

bool matchExpression2(Node* parent) {
    int save = i;
    bool res;

    Node* cur = new Node{Token{EXPRESSION2, "", 0,0}, parent};

    res = matchToken(cur, L_PAR) && matchExpression(cur) && matchToken(cur, R_PAR);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    for (auto it: cur->children) freeNodes(it);
    i = save;

    res = matchUnaryOp(cur) && matchExpression(cur);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    for (auto it: cur->children) freeNodes(it);
    i = save;

    res = matchToken(cur, IDEN) && matchToken(cur,L_BKT) && matchExpression(cur) && matchToken(cur, R_BKT);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    for (auto it: cur->children) freeNodes(it);
    i = save;

    // TODO IDENTIFIER ARG_LIST

    res = matchLiteral(cur);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    for (auto it: cur->children) freeNodes(it);
    i = save;

    res = matchToken(cur, IDEN);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    for (auto it: cur->children) freeNodes(it);
    i = save;

    return false;
}

bool matchExpression(Node* parent) {
    int save = i;
    bool res;

    Node* cur = new Node{Token{EXPRESSION, "", 0,0}, parent};

    res = matchExpression2(cur);
    if (res) {
        
        Node* cur2 = new Node{Token{EXPRESSION, "", 0,0}, parent};
        while (res) { // Match multiple Binary - Experssion sequence
            int save2 = i;
            res = matchBinaryOp(cur2) && matchExpression2(cur2);
            if (res) {
                for (auto it: cur2->children) cur->children.push_back(it);
                cur2->children.clear();
            } else {
                for (auto it: cur2->children) freeNodes(it);
                i = save2;
            }
        }
        parent->children.push_back(cur);
        return true;
    }

    for (auto it: cur->children) freeNodes(it);
    i = save;

    return false;
}

bool matchStatement(Node* parent) {
    int save = i;
    bool res;

    Node* cur = new Node{Token{STATEMENT, "", 0,0}, parent};

    res = matchToken(cur, IDEN) && matchToken(cur, ASSIGN) && matchExpression(cur) && matchToken(cur,SEMI);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    for (auto it: cur->children) freeNodes(it);
    i = save;

    res = matchType(cur) && matchToken(cur, IDEN) && matchToken(cur, ASSIGN) && matchExpression(cur) && matchToken(cur,SEMI);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    for (auto it: cur->children) freeNodes(it);
    i = save;

    return false;
}

bool matchProgram(Node* &cur) {
    int save = i;
    bool res; 
    
    cur = new Node{Token{PROGRAM, "", 0, 0} , NULL};

    res = matchStatement(cur);
    if (res && tokens[i].type == END) {
        return true;
    }
    for (auto it: cur->children) freeNodes(it);
    i = save;

    // Other conditions

    return false;
}

void cleanUp(Node* head);

void parseTokens(vector<Token>& tok) {
    tokens = tok;
    i = 0;

    Node* head;

    bool res = matchProgram(head);

    
    if (res) {
        cout<<"Success\n";
        cleanUp(head);
        printTree(head , 0);
    } else {
        cout<<"Failure";
    }
}


void replaceExpression2(Node* head) {
    if (head->token.type == EXPRESSION2) head->token.type = EXPRESSION;
    for (auto it: head->children) replaceExpression2(it);
}

void fixNestedExpr(Node* head) {
    for (auto it: head->children) fixNestedExpr(it);

    // Remove ()
    if (head->token.type == EXPRESSION && head->children.size() >= 3 
        && head->children[0]->token.type == L_PAR && head->children.back()->token.type == R_PAR) {
        
        head->children.pop_back();
        head->children.erase(head->children.begin());
    } 
    // Reduce expressions containing one expression
    if (head->token.type == EXPRESSION && head->children.size() == 1 && head->children[0]->token.type == EXPRESSION) {
        Node* tmp = head->children[0];
        head->children = head->children[0]->children;
        free(tmp);
    }
}


void cleanUp(Node* head) {
    replaceExpression2(head);
    fixNestedExpr(head);

}

void printTree(Node* cur, int level) {
    Token &t = cur->token;
    for (int i = 0; i < level; i++) printf("  ");
    printf("%s(%s)\n", tokenNames[t.type].c_str(), t.str.c_str());
    for (auto it: cur->children) printTree(it, level + 1);
}



#endif