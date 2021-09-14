#include "./instruction.h"
#include <bits/stdc++.h>
#include "./temptracker.h"
#include "../common.h"
using namespace std;

unordered_map<string, int> funcLabels;
const int STACK_BASE = 0; // The stack base is a label 
int labelNum = 1;

string output = "";

void genFunction(Node* head);
void genExpression(Node* head);
void genBlock(Node* head);
void genStatement(Node* head);
int bool_to_int(string);

vector<Instruction> inst;

string generate(Node* head) {
    // Loop through each function and generate a id for each 
    // + count number of temporaries declared

    for (auto it: head->children) {
        string iden = it->children[1]->token.str;
        funcLabels[iden] = labelNum++;
    }

    // Prepare for main() entry
    inst.push_back(dp(ADD, SP, label(STACK_BASE), imm(2)));
    inst.push_back(dp(MOV, FP, label(STACK_BASE), none()));
    inst.push_back(push(-1));
    inst.push_back(push(-1));


    for (auto it: head->children) {
        genFunction(it);
    }

    return output;
    
}

int st = 0;
TempTracker tt;
TempMapper tmap;
unordered_set<string> usedset; 

void dfsExpr(Node* head) {
    

    if (head->token.type == EXPRESSION && head->token.subtype == IDEN) {
        usedset.insert(head->token.str);
    } else {
        for (auto it: head->children) {
            dfsExpr(it);
        }
    }
}

void dfs(Node* head) {
    
    for (auto it: head->children) {
        
        usedset.clear();
        // DECLASSIGN, ASSIGN, DECLONLY, RETURN, IFELSE, IF, WHILE, FUNCTION_CALL, BLOCK, EMPTY
        if (it->token.subtype == BLOCK) {
            tt.push();
            dfs(it);
            tt.pop();
        } 
        
        if (it->token.subtype == DECLASSIGN || it->token.subtype == DECLONLY) {
            usedset.insert(it->children[0]->children[1]->token.str);
            tt.declared(it->children[0]->children[1]->token.str, st);
        }

        if (it->token.subtype == DECLASSIGN || it->token.subtype == ASSIGN) {
            dfsExpr(it->children[1]);
        }

        if (it->token.subtype == RETURN) dfsExpr(it->children[0]);

        if (it->token.subtype == IF || it->token.subtype == WHILE) {
            dfsExpr(it->children[0]);
            st++;
            tt.push();
            dfs(it->children[1]);
            tt.pop();
        } 

        if (it->token.subtype == IFELSE) {
            dfsExpr(it->children[0]);
            st++;
            tt.push();
            dfs(it->children[1]);
            tt.pop();
            tt.push();
            dfs(it->children[2]);
            tt.pop();
        }

        if (it->token.subtype == FUNCTION_CALL) {
            for (auto it2: it->children[1]->children) {
                dfsExpr(it2);
            }
        }

        for (auto it: usedset) {
            tt.used(it, st);
        }
        st++;
    }
}



void genFunction(Node* head) {
    string iden = head->children[1]->token.str;
    printf("Function: %s\n", iden.c_str());

    int l = funcLabels[iden];

    // Push label to indicate start of function
    inst.push_back(labelI(l));

    st = 0;
    tt = TempTracker();

    tmap = TempMapper(&tt, head->children[2]);

    for (auto it: head->children[2]->children) {
        tt.parameter(it->children[1]->token.str);
        tmap.declared(it->children[1]->token.str); // Just setting declared for the function parameters
    }

    dfs(head->children[3]);
    tt.assignIds();
    tt.printAssignment();

    int num_locals = tt.maxSize;
    
    // Function call code
    inst.push_back(dp(ADD, SP, reg(SP), imm(num_locals))); // SP += num of locals
    
    genBlock(head->children[3]);

}

void genBlock(Node* head) {
    for (auto it: head->children) {
        genStatement(it);
    }
}

void genFuncCall(Node* head) {
    string iden = head->children[0]->token.str;
    int fl = funcLabels[iden];

    for (auto arg: head->children[1]->children) {
        // One by one generate the arguments, and then push to the stack
        genExpression(arg);
        inst.push_back(push(R0)); 
    }

    // Push own FP to stack
    push(FP);

    // Push return address
    // It is not known at this stage yet, so use dynamic label
    int returnAddr = labelNum++;
    inst.push_back(dp(MOV, R0, label(returnAddr), none()));
    push(R0);

    // FP = SP - (num of params + 2)
    inst.push_back(dp(ADD, FP, reg(SP), imm(-(head->children[1]->children.size() + 2))));

    // Set PC to function entry, with dynamic address
    inst.push_back(branch(label(fl)));

    // Put in dummy instruction to indicate return address
    inst.push_back(labelI(returnAddr));
}

void genExpression(Node* head) {
    switch(head->token.type) {
        // Single child expressions: Literals, Variables, Function Calls
        case INT_LIT:
            // Mov the value of the int to R0
            inst.push_back(dp(MOV, R0, imm(stoi(head->token.str)), none())); 
            break;
        case BOOL_LIT:
            // Mov 0 or 1 to R0
            inst.push_back(dp(MOV, R0, imm(bool_to_int(head->token.str)) , none())); 
            break;
        case CHAR_LIT:
            // Move the ASCII char value
            inst.push_back(dp(MOV, R0, imm((int) head->token.str[0]), none()));
            break;
        case IDEN:
            // Get the location of iden in the call stack
            inst.push_back(mem(LDR, R0, reg(FP), tmap.used(head->token.str))); // R0 = [FP + loc]
            break;
        case FUNCTION_CALL:
            genFuncCall(head);
            break;

        // Unary operators: 
        case UADD:
            // result placed in R0 
            genExpression(head->children[0]);
            // Do nothing, for integers +(expression) == expression
            break;
        case USUB:
            genExpression(head->children[0]);
            inst.push_back(dp(MUL, R0, reg(R0), imm(-1)));
            break;
        case NOT:
            genExpression(head->children[0]);
            // Result is a 1 or a 0
            inst.push_back(dp(BXOR, R0, reg(R0), imm(0b1))); // Flip the bit
            break;
        case BNOT:
            genExpression(head->children[0]);
            inst.push_back(dp(BNOT, R0, reg(R1),none()));
            break;
        
        // Arithmetic Binary Operators
        // We assume all operators have a corresponding instruction
        case ADD: case SUB: case MUL: case DIV: case MOD: case BOR: case BAND: case BXOR: case L_SHIFT: case R_SHIFT:
            genExpression(head->children[0]);
            inst.push_back(dp(MOV, R1, reg(R0), none())); // Move temp result to R1
            genExpression(head->children[1]);
            inst.push_back(dp(head->token.type, R0, reg(R1), reg(R0)));
            break;
        
        case OR: case AND:
            genExpression(head->children[0]);
            inst.push_back(dp(MOV, R1, reg(R0), none())); // Move temp result to R1
            genExpression(head->children[1]);
            // use bitwise AND and OR to simulate the logical counterpart
            inst.push_back(dp((head->token.type == OR ? BOR : BAND), R0, reg(R1), reg(R0)));
            break;
        
        case EQ:
            genExpression(head->children[0]);
            inst.push_back(dp(MOV, R1, reg(R0), none())); // Move temp result to R1
            genExpression(head->children[1]);
            inst.push_back(dp(BXOR, R0, reg(R0), reg(R1), true)); // XOR, set flags

            inst.push_back(dp(MOV, R0, imm(0), none()));
            inst.push_back(dp(MOV, R0, imm(1), none(), false, "X1XX")); // Set 1 if Z flag is true
            break;

        case NEQ:
            genExpression(head->children[0]);
            inst.push_back(dp(MOV, R1, reg(R0), none())); // Move temp result to R1
            genExpression(head->children[1]);
            inst.push_back(dp(BXOR, R0, reg(R0), reg(R1), true)); // XOR, set flags

            inst.push_back(dp(MOV, R0, imm(0), none()));
            inst.push_back(dp(MOV, R0, imm(1), none(), false, "X0XX")); // Set 0 if Z flag is false
            break;

        case LT: case LEQ:
            genExpression(head->children[0]);
            inst.push_back(dp(MOV, R1, reg(R0), none())); // Move temp result to R1
            genExpression(head->children[1]);

            if (head->token.type == LEQ) {
                inst.push_back(dp(ADD, R0, reg(R0), imm(1)));
            }

            // Check R1 < R0
            inst.push_back(dp(SUB, R0, reg(R1), reg(R0), true)); 

            inst.push_back(dp(MOV, R0, imm(0), none()));
            inst.push_back(dp(MOV, R0, imm(1), none(), false, "1XXX")); // Set 1 if N flag is true
            break;
        
        case GT: case GEQ:
            genExpression(head->children[0]);
            inst.push_back(dp(MOV, R1, reg(R0), none())); // Move temp result to R1
            genExpression(head->children[1]);

            if (head->token.type == GEQ) {
                inst.push_back(dp(ADD, R1, reg(R1), imm(1)));
            }

            // Check R1 > R0
            inst.push_back(dp(SUB, R0, reg(R0), reg(R1), true)); 

            inst.push_back(dp(MOV, R0, imm(0), none()));
            inst.push_back(dp(MOV, R0, imm(1), none(), false, "1XXX")); // Set 1 if N flag is true
            break;
    }
}

void genStatement(Node* head) {
    // DECLASSIGN, ASSIGN, DECLONLY, RETURN, IFELSE, IF, WHILE, FUNCTION_CALL, BLOCK, EMPTY
    if (head->token.type == DECLASSIGN) {

    }
}

int bool_to_int(string str) {
    if (str == "true") return 1;
    else return 0;
}

void printLabels() {
    for (auto it: funcLabels) {
        printf("%s: %d\n", it.first.c_str(), it.second);
    }
}
