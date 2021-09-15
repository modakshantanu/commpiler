#include <bits/stdc++.h>
#include "../common.h"

using namespace std;

#define MEM_LIMIT 100000
#define INSTRUCTION_LIMIT 100000000

int mem[MEM_LIMIT] = {0};
int reg[16] = {0};
bool N = false;
bool Z = false;
bool C = false;
bool V = false;

vector<string> inst;

int inst_cnt = 0;

void run();
bool isMem(string s);
bool isBranch(string s);
void handleMem(vector<string> &tokens);
void handleBranch(vector<string> &tokens);
void handleDp(vector<string> &toks);
void printRegs();

int main(int argc, char* argv[]) {
      

    if (argc <= 1) {
        printf("Missing input file\n");
        return 0;
    }

    string input = argv[1];
    ifstream file(input);

    if (!file) {
        printf("File not found");
        return 0;
    }

    stringstream buf;
    buf << file.rdbuf();
    string str = buf.str();


    int nInput;
    cout<<"Enter number of input integers: "; cin>>nInput;
    cout<<"Enter input\n";
    for (int i = 0; i < nInput; i++) {
        cin>>mem[READ_BASE + i];
    }

    stringstream stream(str);
    string temp;
    while (getline(stream, temp)) {
        inst.push_back(temp);
    }

    run();

    printf("Total instructions run: %d\n", inst_cnt);
    printf("Contents of output:\n");
    for (int i = 0; i < 16; i++) {
        printf("%d ", mem[WRITE_BASE + i]);
    }
}

void run() {
    while (reg[15] >= 0 && inst_cnt < INSTRUCTION_LIMIT) {

        inst_cnt++;

        // Fetch, update PC
        string in = inst[reg[15]++];
        vector<string> toks;
        stringstream str(in);
        string temp;
        while (str>>temp) toks.push_back(temp);
        

        if (isMem(toks[0])) handleMem(toks);
        else if (isBranch(toks[0])) handleBranch(toks);
        else handleDp(toks);
    }   
    if (inst_cnt == INSTRUCTION_LIMIT) {
        printf("Program terminated due to infinite loop\n");
    } else {
        printf("Program exited successfully\n");
    }
}

int getRegOrImm(string s) {
    if (s[0] == 'R') {
        int regId = stoi(s.substr(1));
        return reg[regId];
    } else return stoi(s);
}

int getRegNo(string s) {
    if (s[0] != 'R') {
        cout<<"Not a register!!\n";
        exit(0);
    }
    return stoi(s.substr(1));
}

bool isMem(string s) {
    return s == "LDR" || s == "STR";
}

bool isBranch(string s) {
    return s == "JMP" || s == "JIF";
} 

// checks if NZCV flags match the conditions string
bool checkCond(string cond) {
    bool res = true;
    if (cond[0] != 'X') res &= (N == (cond[0] == '1'));
    if (cond[1] != 'X') res &= (Z == (cond[1] == '1'));
    if (cond[2] != 'X') res &= (C == (cond[2] == '1'));
    if (cond[3] != 'X') res &= (V == (cond[3] == '1'));
    return res;
}

void setFlags(int num) {
    N = num < 0;
    Z = num == 0;
    // TODO C,V not implemented as it is not needed in code gen
    C = V = 0;
}

void handleDp(vector<string> &toks) {
    

    bool set = false;
    bool conditional = false;
    string op = toks[0];
    if (op.size() == 6) set = conditional = true;
    if (op.size() == 5) conditional = true;
    if (op.size() == 4) set = true;

    if (conditional) {
        string cond = toks[1];
        if (!checkCond(cond)) return;
        // Just remove the conditional string once check is complete
        toks.erase(toks.begin() + 1); 
    }

    int dest = getRegNo(toks[1]);
    int src1 = getRegOrImm(toks[2]);
    int src2 = 0;
    if (toks.size() == 4) src2 = getRegOrImm(toks[3]);

    int res = 0;
    op = op.substr(0,3);

    if (op == "ADD") {
        res = src1 + src2;
    } else if (op == "SUB") {
        res = src1 - src2;
    } else if (op == "MUL") {
        res = src1 * src2;
    } else if (op == "DIV") {
        res = src1 / src2;
    } else if (op == "MOD") {
        res = src1 % src2;
    } else if (op == "NOT") {
        res = ~src1;
    } else if (op == "ORR") {
        res = src1 | src2;
    } else if (op == "AND") {
        res = src1 & src2;
    } else if (op == "XOR") {
        res = src1 ^ src2;
    } else if (op == "LSL") {
        res = src1 << src2;
    } else if (op == "ASR" || op == "LSR") { // For now, logical and arithmetic shift are treated the same
        res = src1 >> src2; 
    } else if (op == "MOV") {
        res = src1;
    }


    reg[dest] = res;
    if (set) setFlags(res);
    // printf("Operation %s on %d, %d, reg[%d] = %d\n", op.c_str(), src1, src2, dest, reg[dest]);



}

void handleBranch(vector<string> &toks) {
    bool condition = true;
    if (toks.size() == 3) {
        condition = checkCond(toks[1]);
    }
    if (!condition) return;
    // JMP by updating PC
    reg[15] = getRegOrImm(toks.back());
}

void handleMem(vector<string> &toks) {
    int dest = getRegNo(toks[1]);
    int addr = getRegOrImm(toks[2]);
    
    if (toks.size() == 4) { // There is an offset
        addr += getRegOrImm(toks[3]);
    }

    if (toks[0] == "LDR") {
        reg[dest] = mem[addr];
        // printf("Loaded reg[%d] = %d from address %d\n", dest, reg[dest], addr);
    } else { // STR
        // printf("Stored mem[%d] = %d from register %d\n", addr, reg[dest], dest);
        mem[addr] = reg[dest];
    }
    // printRegs();
}


void printRegs() {
    for (int i = 0; i < 16; i++) {
        printf("%d ", reg[i]);
    }
    printf("\n");
}