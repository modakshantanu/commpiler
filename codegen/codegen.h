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
    inst.push_back(dp(ADD, SP, label(STACK_BASE), none()));
    inst.push_back(dp(MOV, FP, label(STACK_BASE), none()));
    inst.push_back(dp(MOV, R0, imm(-1), none()));
    inst.push_back(push(R0));
    inst.push_back(push(R0));

    for (auto it: head->children) {
        genFunction(it);
    }
    
    for (auto it: inst) {
        cout<<it.toString()<<endl;
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

        if (it->token.subtype == RETURN && it->children.size() > 0) dfsExpr(it->children[0]);

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
    switch(head->token.subtype) {
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
            inst.push_back(push(R0)); // Temporarily store on stack
            genExpression(head->children[1]);
            inst.push_back(pop(R1)); // Pop 1st result onto R1
            inst.push_back(dp(head->token.subtype, R0, reg(R1), reg(R0)));
            break;
        
        case OR: case AND:
            genExpression(head->children[0]);
            inst.push_back(push(R0));
            genExpression(head->children[1]);
            inst.push_back(pop(R1));
            // use bitwise AND and OR to simulate the logical counterpart
            inst.push_back(dp((head->token.subtype == OR ? BOR : BAND), R0, reg(R1), reg(R0)));
            break;
        
        case EQ:
            genExpression(head->children[0]);
            inst.push_back(push(R0));
            genExpression(head->children[1]);
            inst.push_back(pop(R1));
            inst.push_back(dp(BXOR, R0, reg(R0), reg(R1), true)); // XOR, set flags

            inst.push_back(dp(MOV, R0, imm(0), none()));
            inst.push_back(dp(MOV, R0, imm(1), none(), false, "X1XX")); // Set 1 if Z flag is true
            break;

        case NEQ:
            genExpression(head->children[0]);
            inst.push_back(push(R0));
            genExpression(head->children[1]);
            inst.push_back(pop(R1));
            inst.push_back(dp(BXOR, R0, reg(R0), reg(R1), true)); // XOR, set flags

            inst.push_back(dp(MOV, R0, imm(0), none()));
            inst.push_back(dp(MOV, R0, imm(1), none(), false, "X0XX")); // Set 0 if Z flag is false
            break;

        case LT: case LEQ:
            genExpression(head->children[0]);
            inst.push_back(push(R0));
            genExpression(head->children[1]);
            inst.push_back(pop(R1));

            if (head->token.subtype == LEQ) {
                inst.push_back(dp(ADD, R0, reg(R0), imm(1)));
            }

            // Check R1 < R0
            inst.push_back(dp(SUB, R0, reg(R1), reg(R0), true)); 

            inst.push_back(dp(MOV, R0, imm(0), none()));
            inst.push_back(dp(MOV, R0, imm(1), none(), false, "1XXX")); // Set 1 if N flag is true
            break;
        
        case GT: case GEQ:
            genExpression(head->children[0]);
            inst.push_back(push(R0));
            genExpression(head->children[1]);
            inst.push_back(pop(R1));

            if (head->token.subtype == GEQ) {
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
    int iden_loc;
    int lab1, lab2;
    string iden;
    switch(head->token.subtype) {
        case BLOCK:
            tmap.push();
            genBlock(head);
            tmap.pop();
            break;
        case DECLASSIGN:
            genExpression(head->children[1]);
            // Result is now in R0
            iden = head->children[0]->children[1]->token.str;
            tmap.declared(iden);
            iden_loc = tmap.used(iden);
            inst.push_back(mem(STR, R0, reg(FP), iden_loc)); // Store in stack frame
            break;
        case DECLONLY:
            iden = head->children[0]->token.str;
            tmap.declared(iden);
            break;
        case ASSIGN:
            genExpression(head->children[1]);
            // Result is now in R0
            iden = head->children[0]->token.str;
            iden_loc = tmap.used(iden);
            // printf("iden = %s, loc = %d\n", iden.c_str(), iden_loc);
            inst.push_back(mem(STR, R0, reg(FP), iden_loc)); // Store in stack frame
            break;
        case RETURN:

            if (head->children.size() > 0) {
                genExpression(head->children[0]); // Generate return expression, if any
            }
            inst.push_back(dp(MOV, SP, reg(FP), none())); // SP = FP
            inst.push_back(mem(LDR, FP, reg(SP),tmap.paramList->children.size())); // FP = SP + N
            inst.push_back(mem(LDR, PC, reg(SP),tmap.paramList->children.size() + 1)); // PC = SP + N + 1
            break;
        case IF:
            lab1 = labelNum++;
            genExpression(head->children[0]);
            inst.push_back(dp(ADD, R0, reg(R0), imm(0), true)); // No-op to set flags
        
            inst.push_back(branch(label(lab1), 0, "X1XX")); // JMP IF Z == 1

            tmap.push();
            genBlock(head->children[1]);
            tmap.pop();
            inst.push_back(labelI(lab1));
            break;
        case IFELSE:
            lab1 = labelNum++;
            lab2 = labelNum++;
            genExpression(head->children[0]);
            inst.push_back(dp(ADD, R0, reg(R0), imm(0), true)); // No-op to set flags

            // Jump to else block
            inst.push_back(branch(label(lab1), 0, "X1XX")); // JMP IF Z == 1

            // Code for IF Block
            tmap.push();
            genBlock(head->children[1]);
            tmap.pop();
            inst.push_back(branch(label(lab2))); // Skip else block
            
            // Else block
            inst.push_back(labelI(lab1));
            tmap.push();
            genBlock(head->children[2]);
            tmap.pop();

            inst.push_back(labelI(lab2));
            break;
        case WHILE:
            lab1 = labelNum++;
            lab2 = labelNum++;
            
            inst.push_back(labelI(lab1)); // While label
            genExpression(head->children[0]);
            inst.push_back(dp(ADD, R0, reg(R0), imm(0), true)); // No-op to set flags
            
            inst.push_back(branch(label(lab2), 0, "X1XX")); // Skip loop if false (Z = 1)

            tmap.push();
            genBlock(head->children[1]);
            tmap.pop();
            inst.push_back(branch(label(lab1)));

            // After the while statement
            inst.push_back(labelI(lab2));
            break;
        
        case FUNCTION_CALL:
            genFuncCall(head->children[0]);
            break;
        case EMPTY:
            // Need a No-op to prevent edge case bugs
            inst.push_back(dp(ADD, R0, reg(R0), imm(0)));
            break;
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
