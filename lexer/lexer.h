#include <bits/stdc++.h>
#include "../common.h"
using namespace std;

/*
    SUPPORTED TOKENS
    KEYWORDS 
        INT int
        FLOAT float
        CHAR char
        STR string
        BOOL bool
        IF if
        ELSE else
        FOR for
        WHILE while
        DO do
        PRINT print
        DRAW draw
        RETURN return


    OPERATORS
        (arithmetic)
        ADD
        SUB
        MUL
        DIV
        MOD

        (logical)
        OR
        AND
        NOT
        EQ
        GT
        LT
        GEQ
        LEQ
        NEQ

        (bitwise)
        BOR
        BAND
        BNOT
        BXOR

        ASSIGN

    LITERALS
        CHAR_LIT 'c'
        STR_LIT "ABCD"
        INT_LIT 123
        BOOL_LIT true|false
        FLOAT_LIT 123.456e789

    OTHER
        IDEN idetifier
        L_PAR (
        R_PAR )
        L_BKT [
        R_BKT ]
        L_BRC {
        R_BRC }
        SEMI ;
        COMMA ,
        END 
        WS space, \n or \t
    
        ERR error

*/



bool isKeywordOrIdentifier(string s);
bool isLiteral(string s);
bool isLiteralBreak(char s);
bool isKeywordOrIdentifierBreak(char c);
bool isOperator(string s, char next);
bool isMisc(string s);
bool containsNonWhitespace(string s);

Token getKeyword(string str, int line, int pos);
Token getLiteral(string str, int line, int pos);
Token getOperator(string str, int line, int pos);
Token getMisc(string str, int line, int pos);

vector<Token> tokenize(string in) {
    string buffer = "";
    vector<Token> result;
    int cur_line = 0;
    int cur_pos = 0;
    /*
        Multi Char Tokens-

        Keywords
        Identifiers
        OR , AND , EQ, GEQ, LEQ, NEQ
        LITERALS 

    */
    int i = 0;
    while (i < in.size()) {
        char cur = in[i];
        char next = i < in.size() - 1? in[i+1] : 0;
        buffer.push_back(cur);

        if (isKeywordOrIdentifier(buffer) && isKeywordOrIdentifierBreak(next)) {
            result.push_back(getKeyword(buffer, cur_line, cur_pos));
            buffer = "";
        } else if (isLiteral(buffer) && isLiteralBreak(next)) {
            result.push_back(getLiteral(buffer, cur_line, cur_pos));
            buffer = "";
        } else if (isOperator(buffer, next)) {
            result.push_back(getOperator(buffer, cur_line, cur_pos));
            buffer = "";
        } else if ((cur == ' ' && buffer[0] != '"') || cur == '\t' || cur == '\n') {
            if (containsNonWhitespace(buffer)) {
                result.push_back(Token{ERR, buffer.substr(0, buffer.size() - 1), cur_line, cur_pos});
            }
            if (!(next == ' ' || next == '\t' || next == '\n')) {
                result.push_back(Token{WS, "", cur_line, cur_pos});
                buffer = "";
            }
        } else if (isMisc(buffer)) {
            result.push_back(getMisc(buffer, cur_line, cur_pos));
            buffer = "";
        }

        cur_pos++;
        if (cur == '\n') cur_line++, cur_pos = 0;    
        i++;
    }
    if (buffer.size() > 0) {
        result.push_back(Token{ERR, buffer, cur_line, cur_pos});
    }
    result.push_back(Token{END, "", cur_line, cur_pos});
    return result;
}

bool isKeywordOrIdentifier(string s) {
    // first character is underscore or letter, rest are underscore, letter, or digit
    if (s.size() == 0) return false;
    bool flag = true;
    if (!(isalpha(s[0]) || s[0] == '_')) flag = false;
    for (int i = 1; i < s.size(); i++) {
        if (!(s[i] == '_' || isalnum(s[i]))) flag = false;
    }
    return flag;
}

bool isKeywordOrIdentifierBreak(char c) {
    
    return (!isalnum(c)) && (c != '_');

}

Token getKeyword(string str, int line, int pos) {
    Token res;
    res.line = line;
    res.pos = pos;
    res.str = str;
    if (str == "true" || str == "false") res.type = BOOL_LIT;
    else if (str == "if") res.type = IF;
    else if (str == "else") res.type = ELSE;
    else if (str == "while") res.type = WHILE;
    else if (str == "do") res.type = DO;
    else if (str == "for") res.type = FOR;
    else if (str == "int") res.type = INT;
    else if (str == "float") res.type = FLOAT;
    else if (str == "char") res.type = CHAR;
    else if (str == "string") res.type = STR;
    else if (str == "bool") res.type = BOOL;
    else if (str == "void") res.type = VOID;
    else if (str == "return") res.type = RETURN;
    else res.type = IDEN;

    return res;
}


bool isCharLiteral(string s) {
    if (s.size() != 3 && s.size() != 4) return false;
    if (s.size() == 3) {
        return s[0] == '\'' && s[1] != '\\' && s[2] == '\'';
    } else {
        return s[0] == '\'' && s[1] == '\\' && s[3] == '\'';
    }
}

bool isStringLiteral(string s) {
    if (s.size() < 2) return false;
    if (!(s[0] == '"' && s[s.size() - 1] == '"')) return false;
    for (int i = 1; i < s.size() - 1; i++) {
        if (s[i] == '\n') return false;
        if (s[i] == '\\') {
            if (i == s.size() - 2) {
                // unmatched escape => last " is escaped
                return false;
            } 
            if (s[i+1] == '\n' ) return false;
            i++; // Skip next char 
        }
    }
    return true;
}

bool isIntLiteral(string s) {
    if (s.size() < 1) return false;

    for (int i = 0; i < s.size(); i++) {
        if (!isdigit(s[i])) return false;
    }
    return true;
}

bool isFloatLiteral(string s) {
    if (s.size() < 1) return false;

    bool decimalSeen = false;
    int beforeDecimal = 0;
    int afterDecimal = 0;

    for (int i =0; i < s.size(); i++) {
        if (!isdigit(s[i])) return false;

        if (s[i] == '.') {
            if (decimalSeen) return false; // multiple decimal points
            decimalSeen = true;
        } else {
            if (decimalSeen) afterDecimal++;
            else beforeDecimal++;
        }
    }
    // If decimal not seen, it is a int literal, not float
    if (beforeDecimal == 0 || afterDecimal == 0 || decimalSeen == false) return false;
    return true;
}

bool isLiteral(string s) {
    return isCharLiteral(s) || isStringLiteral(s) || isIntLiteral(s) || isFloatLiteral(s);
}

Token getLiteral(string s, int line, int pos) {
    Token res;
    res.str = s;
    res.line = line;
    res.pos = pos;
    
    if (isCharLiteral(s)) res.type = CHAR_LIT; 
    else if (isStringLiteral(s)) res.type = STR_LIT; 
    else if (isIntLiteral(s)) res.type = INT_LIT; 
    else if (isFloatLiteral(s)) res.type = FLOAT_LIT;
    return res; 
}

bool isLiteralBreak(char c) {
    // String and char literals breaks are not needed as they have ' and "
    // A string / char match guarantees the next one is a break
    // For numeric, it ends in a digit, followed by a non - digit non-decimal point
    // String followed by digit / . is not a valid combo anyways
    return !isdigit(c) && c != '.';
}

bool isOperator(string s, char next) {
    if (s.size() > 2 || s.size() < 1) return false;
    if (s.size() == 1) {
        if ((s == "!" || s == "=") && next != '=') return true;
        if (s == ">" && next != '=' && next != '>') return true;
        if (s == "<" && next != '=' && next != '<') return true;
        if (s == "|" && next != '|') return true;
        if (s == "&" && next != '&') return true;
        if (s == "+" || s == "-" 
                || s == "*" || s == "/" || s == "%" || s == "~" || s == "^") return true;
    } else if (s.size() == 2) {
        if (s == ">=" || s == "<=" || s == "==" || s == "!=" || s == "&&" || s == "||" || s == ">>" || s == "<<") return true;
    }
    return false;
}

map<string, int> m, m2;
Token getOperator(string s, int line, int pos) {
    Token res;
    res.str = s;
    res.line = line;
    res.pos = pos;

    if (m.size() == 0) {
        m.insert({"+",ADD}); 
        m.insert({"-",SUB}); 
        m.insert({"*",MUL}); 
        m.insert({"/",DIV}); 
        m.insert({"%",MOD}); 
        m.insert({"||",OR}); 
        m.insert({"&&",AND}); 
        m.insert({"!",NOT}); 
        m.insert({"==",EQ}); 
        m.insert({">",GT}); 
        m.insert({"<",LT}); 
        m.insert({">=",GEQ}); 
        m.insert({"<=",LEQ}); 
        m.insert({"<<",L_SHIFT}); 
        m.insert({">>",R_SHIFT}); 
        m.insert({"!=",NEQ}); 
        m.insert({"|",BOR}); 
        m.insert({"&",BAND}); 
        m.insert({"~",BNOT}); 
        m.insert({"^",BXOR}); 
        m.insert({"=",ASSIGN}); 
    }

    res.type = m[s];
    return res;
}

bool isMisc(string s) {
    if (s.size() != 1) return false;
    if (s == "(" || s == ")" || s == "{" || s == "}" || s == "[" || s == "]" || s == ";" || s == ",") return true;
    return false;
}

Token getMisc(string s, int line, int pos) {
    Token res;
    res.str = s;
    res.line = line;
    res.pos = pos;

    if (m2.size() == 0) {
        m2.insert({"(",L_PAR}); 
        m2.insert({")",R_PAR}); 
        m2.insert({"[",L_BKT}); 
        m2.insert({"]",R_BKT}); 
        m2.insert({"{",L_BRC}); 
        m2.insert({"}",R_BRC}); 
        m2.insert({";",SEMI}); 
        m2.insert({",",COMMA});    
    }
    res.type = m2[s];
    return res;
}



string to_string(Token &t) {
    return tokenNames[t.type] + ": " + t.str + " @ " + to_string(t.line) + ":" + to_string(t.pos);
}

void printTokens(vector<Token> &tokens) {
    for (auto &t: tokens) {
        cout<<to_string(t)<<endl;
    }
}

bool containsNonWhitespace(string s) {
    for (auto c:s) {
        if (c != ' ' & c != '\t' && c != '\n') return false;
    }
    return false;
}

vector<Token> removeWhitespaces(vector<Token> &toks) {
    vector<Token> result;
    for (auto it: toks) {
        if (it.type != WS) result.push_back(it);
    }
    return result;
}