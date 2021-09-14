#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <bits/stdc++.h>
#include "../common.h"
using namespace std;

enum {DP, MEM, BRANCH, PUSH, POP};
enum {LABEL = 1000, IMM, REG};
enum {R0, R1, R2, R3, FP = 13, SP, PC};



struct Op {
    int type;
    int value;
};

Op reg(int num) {
    return Op{REG, num};
}

Op imm(int num) {
    return Op{IMM, num};
}

Op label(int num) {
    return Op{LABEL, num};
}

Op none() {return Op{NONE, 0};}

string opToString(Op o) {
    if (o.type == NONE) return "";
    if (o.type == REG) return "R" + to_string(o.value);
    if (o.type == LABEL) return "L" + to_string(o.value);
    return to_string(o.value);
}

class Instruction {
public:
    int type;
    int oper;
    bool set = false;
    bool cond = false;
    string cFlags = "";
    int len = 1; 
    int dest;
    Op op1, op2;

    string toString() {
        string res;
        if (type == DP) {
            res += tokenNames[oper];
            if (set) res += "S";
            if (cond) {
                res += "IF ";
                res += cFlags;
            }
            res += " ";
            res += "R" + to_string(dest) +" ";
            res += opToString(op1);
            if (oper != MOV && oper != NOT) {
                res += " ";
                res += opToString(op2); 
            }
        } else if (type == MEM) {
            if (oper == LDR) res += "LDR";
            else res += "STR";
            res += " R" + to_string(dest) + " ";

            // The target memory location
            res += opToString(op1);

            if (op2.type != NONE) {
                res += " " + opToString(op2);
            }
        } else if (type == BRANCH) {
            if (cond) res += "JIF ";
            else res += "JMP ";

            if (cond) res += cFlags + " ";

            res += opToString(op1);
            if (op2.type != NONE) res += " " + opToString(op2);
        } else if (type == PUSH) {

            res += "PUSH R" + to_string(dest);
        } else if (type == POP) {
            res += "POP R" + to_string(dest);
        } else if (type == LABEL) {
            res += "LABEL " + to_string(dest);
        }
        return res;
    } 
};

Instruction dp(int op, int dest, Op op1, Op op2, bool setFlags = false, string condFlags = "") {
    Instruction res;
    res.type = DP;
    res.oper = op;
    res.dest = dest;
    res.op1 = op1;
    res.op2 = op2;
    res.set = setFlags;
    if (condFlags.size() == 4) {
        res.cond = true;
        res.cFlags = condFlags;
    }
    return res;
}

Instruction branch(Op op1, int offset = 0, string flags = "") {
    Instruction res;
    res.type = BRANCH;
    res.op1 = op1;
    if (offset != 0) res.op2 = imm(offset);
    else res.op2 = none();
    if (flags != "") {
        res.cond = true;
        res.cFlags = flags;
    }
    return res;
}

Instruction mem(int op, int dest, Op op1, int offset = 0) {
    Instruction res;
    res.type = MEM;
    res.oper = op;
    res.dest = dest;
    res.op1 = op1;
    if (offset != 0) {
        res.op2 = imm(offset);
    } else {
        res.op2 = none();
    }
    return res;
}

Instruction push(int source) {
    Instruction res;
    res.type = PUSH;
    res.dest = source;
    res.len = 2;
    return res;
}

Instruction pop(int dest) {
    Instruction res;
    res.type = POP;
    res.dest = dest;
    res.len = 2;
    return res;
}

Instruction labelI(int id) {
    Instruction res;
    res.type = LABEL;
    res.len = 0;
    res.dest = id;
    return res;
}

#endif