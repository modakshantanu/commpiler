#ifndef TEMPTRACKER_H
#define TEMPTRACKER_H

#include "../common.h"
#include <bits/stdc++.h>
using namespace std;

struct Var{
    string iden;
    int which = 0;
};

bool operator<(Var a, Var b) {
    return a.iden < b.iden || a.iden == b.iden && a.which < b.which;
}


class TempTracker {
public:
    int maxSize = 0;
    unordered_map<string, int> whichTracker; // How many vars with same name were encountered
    vector<unordered_map<string, int>> scope; 
    map<Var, int> tempId;
    map<Var, int> lastSeen;
    map<Var, int> decl;

    TempTracker() {
        push();
    }

    void push() {
        scope.push_back(unordered_map<string, int>());
    }

    void pop() {
        scope.pop_back();
    }

    void declared(string iden, int st) {
        int w = whichTracker[iden];
        whichTracker[iden] = w + 1;
        Var v = {iden, w};
        scope.back()[iden] = w;
        lastSeen[v] = st;
        decl[v] = st;
    }

    void parameter(string iden) {
        whichTracker[iden]++;
        Var v = {iden , 0};
        tempId[v] = -1;
    }

    void used(string iden, int st) {
        int w = -1;
        for (int i = scope.size() - 1; i >= 0; i--) {
            if (scope[i].count(iden)) {
                w = scope[i][iden];
                break;
            }
        }
        if (w == -1) {
            // It is a function parameter, ignore
            return;
        }
        Var v = {iden, w};
        lastSeen[v] = st;
    }


    void assignIds() {
        priority_queue<pair<int, Var>, vector<pair<int, Var>>, greater<pair<int, Var>>> start, stop;
        for (auto it: decl) {
            start.push({it.second, it.first});
        }

        for (auto it: lastSeen) {
            stop.push({it.second, it.first});
        }


        stack<int> freePool;


        
        int i = 0;
        // return;
        while (!stop.empty()) {

            while (!start.empty() && i >= start.top().first) {
                Var v = start.top().second;
                int tid = 0;
                if (freePool.size() > 0) {
                    tid = freePool.top();
                    freePool.pop();
                } else {
                    tid = maxSize++;
                }

                tempId[v] = tid;
                start.pop();
            }
            while (!stop.empty() && i >= stop.top().first) {
                Var v = stop.top().second;
                freePool.push(tempId[v]);
                stop.pop();
            }
            i++;
        }
    }


    void printAssignment() {
        for (auto it:tempId) {
            printf("%s(%d) : %d\n", it.first.iden.c_str(), it.first.which, it.second);
        }
    }
};

// Once assignment of temps is fixed, this class handles actual mapping during parsing of the program.
class TempMapper {
public:
    unordered_map<string, int> whichTracker; 
    vector<unordered_map<string, Var>> scope; 
    TempTracker* tempMap;
    Node* paramList;

    TempMapper(TempTracker* map = NULL, Node* paramList = NULL) {
        push();
        tempMap = map;
    }

    void push() {
        scope.push_back(unordered_map<string, Var>());
    }

    void pop() {
        scope.pop_back();
    }
    

    void declared(string iden) {
        int w = whichTracker[iden];
        whichTracker[iden] = w + 1;
        Var v = {iden, w};
        scope.back()[iden] = {iden, w};
    }

    int used(string iden) {
        for (int i = scope.size() - 1; i >= 0; i--) {
            if (scope[i].count(iden) != 0) {
                auto var = scope[i][iden];
                int tempId = tempMap->tempId[var];
                if (tempId == -1) {
                    for (int j = 0; j < paramList->children.size(); j++) {
                        if (paramList->children[j]->token.str == iden) return j;
                    }
                } else {
                    return tempId + paramList->children.size() + 2;
                }
            }
        }
        return 0;
    }

};

#endif