#ifndef SEMERROR_H
#define SEMERROR_H

#include <bits/stdc++.h>
using namespace std;

void err(int line, int pos) {
    if (line == -1) {
        printf("Semantic Error at unknown location\n");
    } else {
        printf("Semantic error at line %d pos %d\n", line + 1, pos + 1);
    }
    exit(0);
}

#endif