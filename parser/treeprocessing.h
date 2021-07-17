#ifndef TREEPROC_H
#define TREEPROC_H

#include "../common.h"

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

void fixFunctionAndIndexed(Node* head) {
    for (auto it: head->children) fixFunctionAndIndexed(it);

    if (head->token.type == INDEXED) {
        head->children = {head->children[0] , head->children[2]};
    }

    if (head->token.type == ARG_LIST) {
        vector<Node*> updated;
        for (auto it: head->children) {
            if (it->token.type == EXPRESSION) updated.push_back(it);
        }
        head->children = updated;
    }
}

void fixNonExpressions(Node* head) {
    for (auto it: head->children) fixNonExpressions(it);

    if (head->token.type == PARAM_LIST) {
        // Strip paranthesis and commas
        vector<Node*> updated;
        for (auto it: head->children) {
            if (it->token.type == DECLARATION) updated.push_back(it);
            else free(it); // No need to recursively free
        } 
        head->children = updated;
    } else if (head->token.type == ARG_LIST) {
        // Strip paren and commas
        vector<Node*> updated;
        for (auto it: head->children) {
            if (it->token.type == EXPRESSION) updated.push_back(it);
            else free(it); // No need to recursively free
        } 
        head->children = updated;
    } else if (head->token.type == INDEXED) {
        free(head->children[1]);
        free(head->children[3]);
        head->children = {head->children[0], head->children[2]};
    } else if (head->token.type == STATEMENT) {
        if (head->children[0]->token.type == BLOCK) {
            // Transfer all children to parent
            auto temp = head->children[0];
            head->children = temp->children;
            free(temp);
            head->token.subtype = BLOCK;
        } else if (head->children[0]->token.type == RETURN) {
            free(head->children[0]);
            free(head->children.back());
            if (head->children.size() == 3) head->children = {head->children[1]};
            else head->children = {};
            head->token.subtype = RETURN;
        } else if (head->children.size() > 2 && head->children[1]->token.type == ASSIGN && head->children[0]->token.type == IDEN) {
            free(head->children[1]); free(head->children[3]);
            head->children = {head->children[0], head->children[2]};
            head->token.subtype = ASSIGN;
        } else if (head->children.size() > 2 && head->children[1]->token.type == ASSIGN && head->children[0]->token.type == DECLARATION) {
            free(head->children[1]); free(head->children[3]);
            head->children = {head->children[0], head->children[2]};
            head->token.subtype = DECLASSIGN;
        } else if (head->children.size() == 2 && head->children[0]->token.type == DECLARATION) {
            free(head->children[1]); head->children.pop_back();
            head->token.subtype = DECLONLY;
        } else if (head->children[0]->token.type == FUNCTION_CALL) {
            free(head->children[1]); head->children.pop_back();
            head->token.subtype = FUNCTION_CALL;
        } else if (head->children[0]->token.type == IF && head->children.size() == 7) {
            free(head->children[0]); free(head->children[1]); free(head->children[3]); free(head->children[5]);
            head->children = {head->children[2],  head->children[4], head->children[6]};
            head->token.subtype = IFELSE;
        } else if (head->children[0]->token.type == IF && head->children.size() == 5) {
            free(head->children[0]); free(head->children[1]); free(head->children[3]); 
            head->children = {head->children[2],  head->children[4]};
            head->token.subtype = IF;
        } else if (head->children[0]->token.type == WHILE) {
            free(head->children[0]); free(head->children[1]); free(head->children[3]); 
            head->children = {head->children[2],  head->children[4]};
            head->token.subtype = WHILE;
        } else if (head->children[0]->token.type == SEMI) {
            free(head->children[0]); 
            head->children = {};
            head->token.subtype = EMPTY;
        }
    }
}

int expressionType(int t) {
    if (t ==  INT_LIT || t == CHAR_LIT || t == BOOL_LIT || t == STR_LIT || t == FLOAT_LIT) return LITERAL;
    
    return t; // Functioncall Identifer Indexed
}


// https://en.cppreference.com/w/c/language/operator_precedence
int precedence(int op) {
    switch(op) {
        case MUL: case DIV: case MOD: return 3;
        case ADD: case SUB: return 4;
        case L_SHIFT: case R_SHIFT: return 5;
        case LT: case GT: case LEQ: case GEQ: return 6;
        case EQ: case NEQ: return 7;
        case BAND: return 8;
        case BXOR: return 9;
        case BOR: return 10;
        case AND: return 11;
        case OR: return 12;
        case L_PAR: 100; // Dummy char with lowest prio
    }
    return 100;
}

void fixExpressions(Node* head) {
    
    for (auto it: head->children) fixExpressions(it);
    auto& children = head->children;

    if (head->token.type != EXPRESSION) return;

    if (children.size() == 1) {

        int ctype = children[0]->token.type;
        int csubtype = children[0]->token.subtype;
        // Terminal Nodes
        int e = expressionType(ctype);


        if (e == LITERAL || e == IDEN || e == FUNCTION_CALL || e == INDEXED) {
            head->token = children[0]->token;
            head->token.subtype = head->token.type;
            head->token.type = EXPRESSION;
            auto tmp = children[0];
            head->children = children[0]->children;
            free(tmp);
            return;
        }
        cout<<ctype<<endl;

    } else if (children.size() == 2) {
        // Unary Operators
        int op = children[0]->token.type;
        if (op == ADD) op = UADD;
        if (op == SUB) op = USUB;

        if ( op == UADD) {
            // Set head = child[1], ignore the operation
            head->token = children[1]->token;
            head->token.subtype = head->token.subtype;
            head->token.type = EXPRESSION;
            auto tmp = children[1];
            children = children[1]->children;
            free(children[0]); free(tmp);
            return;
        } else if ((op == USUB && children[1]->token.subtype == USUB) || (op == NOT && children[1]->token.subtype == NOT)) {
            // Set head = child[1]->child[0] (double negation)
            head->token = children[1]->children[0]->token;
            head->token.subtype = head->token.subtype;
            head->token.type = EXPRESSION;
            auto tmp = children[1];
            children = children[1]->children[0]->children;
            free(children[0]); free(tmp->children[0]); free(tmp);
            return;
        }
        head->token.subtype = op;
        free(children[0]);
        children = {children[1]};
        return;
    } else {
        vector<Node*> postfix;
        vector<Node*> operators;

        Node dummy = {Token{L_PAR, "", 0,0}};
        operators.push_back(&dummy);

        for (auto it: children) {
            if (it->token.type == EXPRESSION) postfix.push_back(it);
            else {
                while (precedence(it->token.type) >= precedence(operators.back()->token.type)) {
                    postfix.push_back(operators.back());
                    operators.pop_back();
                }
                operators.push_back(it);
            } 
        }
        while (operators.size()) {
            postfix.push_back(operators.back());
            operators.pop_back();
        }

        postfix.pop_back(); // remove (

        children.clear();

        for (auto it: postfix) {
            if (it->token.type == EXPRESSION) {
                children.push_back(it);
            } else {
                auto b = children.back(); children.pop_back();
                auto a = children.back(); children.pop_back();
                it->token.subtype = it->token.type;
                it->token.type = EXPRESSION;
                it->children = {a,b};
                children.push_back(it);
            }
        }

        head->token = children[0]->token;
        auto tmp = children[0];
        children = tmp->children;
        free(tmp);
    }
}

void cleanUp(Node* head) {
    replaceExpression2(head);
    fixNestedExpr(head);
    fixNonExpressions(head);
    fixExpressions(head);
}

void printTree(Node* cur, int level) {
    Token &t = cur->token;
    for (int i = 0; i < level; i++) printf("  ");
    printf("%s:%s(%s)\n", tokenNames[t.type].c_str(), tokenNames[t.subtype].c_str(), t.str.c_str());
    for (auto it: cur->children) printTree(it, level + 1);
}

#endif