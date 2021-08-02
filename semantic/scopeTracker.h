#ifndef SCOPETRACKER_H
#define SCOPETRACKER_H

#include <bits/stdc++.h>
#include "../common.h"

using namespace std;

class VarTracker {
public:
    vector<unordered_map<string, int>> varStack;

    VarTracker() {
        varStack.push_back(unordered_map<string, int>());
    }
    
    void addIdentifier(string id, int type) {
        varStack.back()[id] = type;
    } 

    int getType(string id) {
        for (int i = varStack.size() - 1; i >= 0; i--) {
            if (varStack[i].count(id)) return varStack[i][id];
        }
        return ERR;
    } 

    bool inCurScope(string id) {
        return varStack.back().count(id) > 0;
    }

    void pushScope() {{
        varStack.push_back(unordered_map<string, int>());
    }}

    void popScope() {
        varStack.pop_back();
    }

    void print() {
        for (int i = 0; i < varStack.size(); i++) {
            printf("Depth %d\n", i+1);
            for (auto it: varStack[i]) {
                printf("\t%s : %s\n", it.first.c_str(), tokenNames[it.second].c_str());
            }
        }
    }
};

class FuncTracker {
public:
    map<pair<string, vector<int>>, int> funcMap;

  
    void addFunc(string id, vector<int> params, int returnType) {
        funcMap[{id, params}] = returnType;
    } 

    int funcType(string id, vector<int> params) {
        if (funcMap.count({id, params}) > 0) return funcMap[{id, params}];
        return ERR;
    } 
    
    void print() {
        for (auto it: funcMap) {
            auto id = it.first.first;
            auto param = it.first.second;
            auto type = it.second;

            printf("%s(", id.c_str());
            for (int i = 0; i < param.size(); i++) {
                printf("%s", tokenNames[param[i]].c_str());
                if (i != param.size() - 1) printf(", ");
            }
            printf(") : %s\n", tokenNames[type].c_str());

        }
    }
};



#endif