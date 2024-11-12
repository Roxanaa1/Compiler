#pragma once
 

enum {
    ID,
    // Cuvinte cheie
    VAR,
    FUNCTION,
    IF,
    ELSE,
    WHILE,
    END,
    RETURN,
    TYPE_INT,
    TYPE_REAL,
    TYPE_STR,
    // Constante
    INT,
    REAL,
    STR,
    // Delimitatori
    COMMA,
    COLON,
    SEMICOLON,
    LPAR,
    LESSEQ,
    RPAR,
    FINISH,
    // Operatorii
    ADD,
    SUB,
    MUL,
    DIV,
    AND,
    OR,
    NOT,
    ASSIGN,
    EQUAL,
    NOTEQ,
    LESS,
    GREATER,
    GREATEREQ,
    // Elemente de ignorat (spatii si comentarii)
    SPACE,
    COMMENT
};
#define MAX_STR		127 //maxim 127 de caractere + terminatorul de sir 


typedef struct {
    int code;		// ID, TYPE_INT, ...
    int line;		// the line from the input file
    union {
        char text[MAX_STR + 1];		// the chars for ID, STR
        int i;		// the value for INT
        double r;		// the value for REAL
    };
}Token;

#define MAX_TOKENS		4096 
Token tokens[];
int nTokens; 
const char* tkCodeName(int code);


extern void tokenize(const char* pch);
extern void showTokens();
