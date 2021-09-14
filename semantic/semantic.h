#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <bits/stdc++.h>
#include "../common.h"
#include "scopeTracker.h"
#include "semanticerror.h"

using namespace std;
FuncTracker ft;
VarTracker vt;

void checkFunction(Node* head);
void checkBlock(Node* head); 
void checkStatement(Node* head); 
int checkExpression(Node* head); 
void checkDeclaration(Node * head);

void checkSemantics(Node* head) {
    ft = FuncTracker();
    vt = VarTracker();

    // Populate function data
    for (auto it: head->children) {
        int type = it->children[0]->token.type;
        string iden = it->children[1]->token.str;
        vector<int> params;
        for (auto p: it->children[2]->children) {
            params.push_back(p->children[0]->token.type);
        }
        ft.addFunc(iden, params, type);
    }

    for (Node* it: head->children) {
        checkFunction(it);
    }

}

int returnType;

Node* getDefaultReturn(Node* parent) {
    Node* res = new Node{Token{STATEMENT, "", 0,0, RETURN}, parent};
    string literal;
    int litType;
    if (returnType == INT) {
        literal = "0";
        litType = INT_LIT;
    } else if (returnType == FLOAT) {
        literal = "0.0";
        litType = FLOAT_LIT;
    } else if (returnType == CHAR) {
        literal = "'\\0'";
        litType = CHAR_LIT;
    } else if (returnType == STR) {
        literal = "\"\"";
        litType = STR_LIT;
    } else {
        literal = "";
    }


    if (returnType != VOID) {
        res->children.push_back(new Node{Token{EXPRESSION, literal, 0,0, litType}, res});
    }

    return res;
}

void checkFunction(Node* head) {

    vt.pushScope();

    // Process function parameters
    for (auto decl: head->children[2]->children) {
        int type = decl->children[0]->token.type;
        string iden = decl->children[1]->token.str;
        
        // Function Arguments cant be void type
        if (type == VOID) {
            err(decl->children[0]->token.line, decl->children[0]->token.pos);
        }

        // Check if not in scope
        if (vt.inCurScope(iden)) {
            err(decl->children[1]->token.line, decl->children[1]->token.pos);
        }

        // Add identifier to scope
        vt.addIdentifier(iden, type);
    }

    returnType = head->children[0]->token.type;

    Node* s = head->children[3];
    // Add a dummy return to the end for good measure
    s->children.push_back(getDefaultReturn(s));

    checkBlock(s);
    
    vt.popScope();
}


void checkBlock(Node* head) {
    for (auto s: head->children) {
        checkStatement(s);
    }
}

void checkStatement(Node* head) {

    int subType = head->token.subtype;
    vector<Node*>& children = head->children;

    Node *decl, *expr, *func;
    Token t;
    string iden;
    int type, exprType, rType;
    vector<int> argTypes;
    
    switch(subType) {
        case DECLASSIGN:
            decl = children[0];
            expr = children[1];
            iden = decl->children[1]->token.str;
            t = decl->children[0]->token;
            type = t.type;
            
            // Not already declared
            if (vt.inCurScope(iden)) err(t.line, t.pos);
            vt.addIdentifier(iden, type);

            // type(LHS) == type(RHS)
            exprType = checkExpression(expr);
            if (exprType != type) err(t.line, t.pos);
            break;

        case ASSIGN:

            iden = children[0]->token.str;
            type = vt.getType(iden);
            expr = children[1];

            // Check if var declared & check if expression type matches
            if (!(type != ERR && checkExpression(expr) == type)) {
                err(children[0]->token.line, children[0]->token.pos);
            }     
            break;

        case DECLONLY:
            decl = children[0];
            iden = decl->children[1]->token.str;
            t = decl->children[0]->token;
            type = t.type;
            
            // Not already declared
            if (vt.inCurScope(iden)) err(t.line, t.pos);
            vt.addIdentifier(iden, type);
            break;

        case RETURN:
            if (children.size() == 0) {
                rType = VOID;
            } else {
                rType = checkExpression(children[0]);
            }
            if (rType != returnType) err(-1,-1);
            break;

        case IFELSE:
            if (checkExpression(children[0]) != BOOL) err(-1,-1);
            checkStatement(children[1]);
            checkStatement(children[2]);
            break;

        case IF:
        case WHILE:
            if (checkExpression(children[0]) != BOOL) err(-1,-1);
            checkStatement(children[1]);
            break;

        case FUNCTION_CALL:
            func = children[0];
            iden = func->children[0]->token.str;

            for (Node* a: func->children[1]->children) {
                argTypes.push_back(checkExpression(a));
            }

            if (ft.funcType(iden, argTypes) == ERR) err(func->children[0]->token.line, func->children[0]->token.pos);
            break;

        case BLOCK:
            vt.pushScope();
            checkBlock(head);
            vt.popScope();
            break;

        case EMPTY:
            break;
    }
}

int checkExpression(Node* head) {
    Node *c1, *c2;
    if (head->children.size() > 0) c1 = head->children[0];
    if (head->children.size() > 1) c2 = head->children[1];
    
    int subType = head->token.subtype;
    int type;
    string iden;
    vector<int> argTypes;
    
    // All expressions with no children
    switch(subType) {
        case IDEN:
            type = vt.getType(head->token.str);
            if (type == ERR) err(head->token.line, head->token.pos);
            return type;
        
        // TODO Indexed

        case INT_LIT: return INT;
        case FLOAT_LIT: return FLOAT;
        case BOOL_LIT: return BOOL;
        case CHAR_LIT: return CHAR;
        case STR_LIT: return STR;

        case FUNCTION_CALL:
            iden = c1->token.str;

            for (Node* a: c2->children) {
                argTypes.push_back(checkExpression(a));
            }

            type = ft.funcType(iden, argTypes);
            if (type == ERR) err(c1->token.line, c1->token.pos);
            return type;
    }

    // Unary
    if (head->children.size() == 1) {
        int op = head->token.subtype;
        int type = checkExpression(c1);

        switch(op) {
            case USUB: case UADD: case BNOT:    
                if (type == INT) return INT;
                break;
            case NOT:   
                if (type == BOOL) return BOOL;
                break;
        }
        err(head->token.line, head->token.pos);
    } else { // Binary
        int op = head->token.subtype;
        int t1 = checkExpression(c1);
        int t2 = checkExpression(c2);

        switch(op) {
            // INT - INT -> INT
            case MOD: case L_SHIFT: case R_SHIFT: case BOR: case BAND: case BXOR:
                if (t1 == INT && t2 == INT) return INT;
                break;
            // NUM - NUM -> NUM
            case ADD: case SUB: case MUL: case DIV: 
                if (t1 == t2 && (t1 == INT || t1 == FLOAT)) return t1;
                break;

            // NUM - NUM -> BOOL            
            case GT: case LT: case GEQ: case LEQ:
                if (t1 == t2 && (t1 == INT || t1 == FLOAT)) return BOOL;
                break;

            // BOOL - BOOL -> BOOL
            case OR: case AND:
                if (t1 == BOOL && t2 == BOOL) return BOOL;
                break;
            
            // ANY - ANY -> BOOL
            case EQ: case NEQ:
                if (t1 == t2) return BOOL;
                break;
            
        }
        err(head->token.line, head->token.pos);
    }

    
    // Should never reach here
    err(head->token.line, head->token.pos);
    return ERR;
}

#endif