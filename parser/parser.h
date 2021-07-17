#ifndef PARSER_H
#define PARSER_H

#include "../common.h"
#include "./treeprocessing.h"



vector<Node*> temp;

vector<Token> tokens;
int i = 0;

bool matchExpression(Node* parent);
bool matchStatement(Node* parent);

void freeNodes(Node* head) {
    if (head == NULL) return;
    for (auto it: head->children) free(it);
    free(head);
}

void freeArray(Node* head) {
    for (auto it: head->children) freeNodes(it);
    head->children.clear();
}

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
        || t.type == GT || t.type == LEQ || t.type == GEQ || t.type == NEQ || t.type == BOR || t.type == BAND || t.type == BNOT || t.type == BXOR || t.type == L_SHIFT || t.type == R_SHIFT;

    if (res) {
        parent->children.push_back(new Node{t, parent});
    }
    return res;
    
}

bool matchType(Node* parent) {
    Token t = tokens[i++];
    bool res = t.type == INT || t.type == STR || t.type == FLOAT || t.type == BOOL || t.type == CHAR || t.type == VOID;
    if (res) {
        parent->children.push_back(new Node{t, parent});
    }
    return res;
}

bool matchDeclaration(Node* parent) {
    int save = i;
    Node* cur = new Node{Token{DECLARATION, "", 0, 0}, parent};
    bool res = matchType(cur) && matchToken(cur, IDEN);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }

    i = save;
    freeNodes(cur);
    return false;
}

bool matchArglist(Node* parent) {
    int save = i;
    bool res;
    Node* cur = new Node{Token{ARG_LIST, "", 0, 0}, parent};

    res = matchToken(cur, L_PAR);
    if (!res) {
        i = save;
        freeNodes(cur);
        return false;
    }

    // No arguments
    if (tokens[i].type == R_PAR) {
        matchToken(cur, R_PAR);
        parent->children.push_back(cur);
        return true;
    }

    res = matchExpression(cur);
    if (res) {
        Node* cur2 = new Node{Token{ARG_LIST, "", 0,0}, parent};
        while (res) { // Match multiple Binary - Experssion sequence
            int save2 = i;
            res = matchToken(cur2, COMMA) && matchExpression(cur2);
            if (res) {
                for (auto it: cur2->children) cur->children.push_back(it);
                cur2->children.clear();
            } else {
                freeArray(cur2);
                i = save2;
            }
        }
    }

    if (tokens[i].type == R_PAR) {
        matchToken(cur, R_PAR);
        parent->children.push_back(cur);
        return true;
    }

    freeNodes(cur);
    return false;
}

bool matchParamlist(Node* parent) {
    int save = i;
    bool res;
    Node* cur = new Node{Token{PARAM_LIST, "", 0, 0}, parent};

    res = matchToken(cur, L_PAR);
    if (!res) {
        i = save;
        freeNodes(cur);
        return false;
    }

    // No arguments
    if (tokens[i].type == R_PAR) {
        matchToken(cur, R_PAR);
        parent->children.push_back(cur);
        return true;
    }

    res = matchDeclaration(cur);
    if (res) {
        Node* cur2 = new Node{Token{ARG_LIST, "", 0,0}, parent};
        while (res) { // Match multiple Binary - Experssion sequence
            int save2 = i;
            res = matchToken(cur2, COMMA) && matchDeclaration(cur2);
            if (res) {
                for (auto it: cur2->children) cur->children.push_back(it);
                cur2->children.clear();
            } else {
                freeArray(cur2);
                i = save2;
            }
        }
    }

    if (tokens[i].type == R_PAR) {
        matchToken(cur, R_PAR);
        parent->children.push_back(cur);
        return true;
    }

    freeNodes(cur);
    return false;
}

bool matchFunctionCall(Node* parent) {
    int save = i;
    bool res;
    Node* cur = new Node{Token{FUNCTION_CALL, "", 0, 0}, parent};

    res = matchToken(cur, IDEN) && matchArglist(cur);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;

    freeNodes(cur);
    return false;
}


bool matchFunctionDecl(Node* parent) {
    int save = i;
    bool res;
    Node* cur = new Node{Token{FUNCTION_DECL, "", 0, 0}, parent};

    res = matchType(cur) && matchToken(cur, IDEN) && matchParamlist(cur) && matchStatement(cur);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;

    freeNodes(cur);
    return false;
}


bool matchIndexed(Node* parent) {
    int save = i;
    Node* cur = new Node{Token{INDEXED, "", 0, 0}, parent};
    if (matchToken(cur, IDEN) && matchToken(cur, L_BKT) && matchExpression(cur) && matchToken(cur, R_BKT)) {
        parent->children.push_back(cur);
        return true;
    }

    freeNodes(cur);
    i = save;
    return false;
}

bool matchExpression2(Node* parent) {
    int save = i;
    bool res;

    Node* cur = new Node{Token{EXPRESSION2, "", 0,0}, parent};

    res = matchToken(cur, L_PAR) && matchExpression(cur) && matchToken(cur, R_PAR);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;

    res = matchUnaryOp(cur) && matchExpression2(cur);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;

    res = matchIndexed(cur);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;

    res = matchFunctionCall(cur);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;

    res = matchLiteral(cur);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;

    res = matchToken(cur, IDEN);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;

    freeNodes(cur);
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
                freeArray(cur2);
                i = save2;
            }
        }
        parent->children.push_back(cur);
        return true;
    }

    freeArray(cur);
    i = save;

    freeNodes(cur);
    return false;
}

bool matchBlock(Node* parent) {
    int save = i;
    bool res;
    Node* cur = new Node{Token{BLOCK, "", 0, 0}, parent};

    res = matchToken(cur, L_BRC);
    if (!res) {
        i = save;
        freeNodes(cur);
        return false;
    }

    res = matchStatement(cur);
    if (res) {
        Node* cur2 = new Node{Token{BLOCK, "", 0,0}, parent};
        while (res) {
            int save2 = i;
            res = matchStatement(cur2);
            if (res) {
                for (auto it: cur2->children) cur->children.push_back(it);
                cur2->children.clear();
            } else {
                freeArray(cur2);
                i = save2;
            }
        }
    } else {
        i = save;
        freeNodes(cur);
        return false;
    }

    if (tokens[i].type == R_BRC) {
        matchToken(cur, R_BRC);
        
        // Remove braces
        cur->children.pop_back();
        cur->children.erase(cur->children.begin());

        parent->children.push_back(cur);
        return true;
    }

    freeNodes(cur);
    return false;
}

bool matchStatement(Node* parent) {
    int save = i;
    bool res;

    Node* cur = new Node{Token{STATEMENT, "", 0,0}, parent};

    // Assignment
    res = matchToken(cur, IDEN) && matchToken(cur, ASSIGN) && matchExpression(cur) && matchToken(cur,SEMI);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;

    // Declaration and Assignment
    res = matchDeclaration(cur) && matchToken(cur, ASSIGN) && matchExpression(cur) && matchToken(cur,SEMI);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;

    // Declaration
    res = matchDeclaration(cur) && matchToken(cur,SEMI);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;

    // Return void
    res = matchToken(cur, RETURN) && matchToken(cur,SEMI);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;

    // Return 
    res = matchToken(cur, RETURN) && matchExpression(cur) && matchToken(cur,SEMI);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;

    // If Else
    res = matchToken(cur, IF) && matchToken(cur, L_PAR) && matchExpression(cur) && matchToken(cur, R_PAR) && matchStatement(cur) && matchToken(cur, ELSE) && matchStatement(cur);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;

    // If
    res = matchToken(cur, IF) && matchToken(cur, L_PAR) && matchExpression(cur) && matchToken(cur, R_PAR) && matchStatement(cur);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;


    // While
    res = matchToken(cur, WHILE) && matchToken(cur, L_PAR) && matchExpression(cur) && matchToken(cur, R_PAR) && matchStatement(cur);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;

    // Block
    res = matchBlock(cur);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;

    // Function Call
    res = matchFunctionCall(cur) && matchToken(cur, SEMI);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;


    // Semicolon
    res = matchToken(cur,SEMI);
    if (res) {
        parent->children.push_back(cur);
        return true;
    }
    freeArray(cur);
    i = save;

    freeNodes(cur);
    return false;
}

bool matchProgram(Node* &cur) {
    int save = i;
    bool res; 
    
    cur = new Node{Token{PROGRAM, "", 0, 0} , NULL};

    while (tokens[i].type != END) {
        res = matchFunctionDecl(cur);

        if (!res) {
            freeArray(cur);
            return false;
        }
    }

    return true;
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






#endif