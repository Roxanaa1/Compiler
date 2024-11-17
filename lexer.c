#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include "lexer.h"
#include "utils.h"

Token tokens[MAX_TOKENS];
int nTokens;
int line = 1;

Token* addTk(int code) {
    if (nTokens == MAX_TOKENS)
        err("Prea multe tokenuri");
    if (code < 0)
        err("Invalid token code");

    Token* tk = &tokens[nTokens];
    tk->code = code;
    tk->line = line;
    nTokens++;
    return tk;
}

char* copyn(char* dst, const char* begin, const char* end) {
    if (end < begin)
        err("Invalid string range");

    char* p = dst;

    if (end - begin > MAX_STR)
        err("String prea lung");

    while (begin != end)
        *p++ = *begin++;
    *p = '\0';
    return dst;
}

void tokenize(const char* pch) {
    const char* start;
    Token* tk;
    char buf[MAX_STR + 1];

    for (;;) {
        switch (*pch) {
        case ' ': case '\t': case '\r': case '\n':
            if (*pch == '\n') line++;
            pch++;
            break;
        case '\0':
            addTk(FINISH);
            return;
        case ',':
            addTk(COMMA);
            pch++;
            break;
        case ':':
            addTk(COLON);
            pch++;
            break;
        case ';':
            addTk(SEMICOLON);
            pch++;
            break;
        case '(':
            addTk(LPAR);
            pch++;
            break;
        case ')':
            addTk(RPAR);
            pch++;
            break;
        case '+':
            addTk(ADD);
            pch++;
            break;
        case '-':
            addTk(SUB);
            pch++;
            break;
        case '*':
            addTk(MUL);
            pch++;
            break;
        case '/':
            addTk(DIV);
            pch++;
            break;
        case '=':
            if (pch[1] == '=') {
                addTk(EQUAL);
                pch += 2;
            }
            else {
                addTk(ASSIGN);
                pch++;
            }
            break;
        case '!':
            if (pch[1] == '=') {
                addTk(NEQUAL);
                pch += 2;
            }
            else {
                addTk(NOT);
                pch++;
            }
            break;
        case '<':
            if (pch[1] == '=') {
                addTk(LESS_EQUAL);
                pch += 2;
            }
            else {
                addTk(LESS);
                pch++;
            }
            break;
        case '>':
            if (pch[1] == '=') {
                addTk(GREATER_EQUAL);
                pch += 2;
            }
            else {
                addTk(GREATER);
                pch++;
            }
            break;
        case '&':
            if (pch[1] == '&') {
                addTk(AND);
                pch += 2;
            }
            else {
                err("Error at AND operator: Use '&&' instead of '&' at line %d", line);
            }
            break;
        case '|':
            if (pch[1] == '|') {
                addTk(OR);
                pch += 2;
            }
            else {
                err("Error at OR operator: Use '||' instead of '|' at line %d", line);
            }
            break;

        case '#':  // Comentariu
            while (*pch != '\n' && *pch != '\0') {
                if (pch[1] == '#') {
                    err("Invalid comment, please enter only one '#' at line %d", line);
                }/*else if(*pch == '#' && pch[1] == ' '){
                    err("Invalid comment, please enter a space after '#' or add content at line %d", line);
                }*/
                pch++;
            }
            break;
        case '"':
            start = ++pch;
            while (*pch != '"' && *pch != '\0')
                pch++;
            if (*pch == '"') {
                char* text = copyn(buf, start, pch);
                tk = addTk(STR);
                strcpy(tk->text, text);
                pch++;
            }
            else {
                err("Unterminated string at line %d", line);
            }
            break;
        default:
            if (isalpha(*pch) || *pch == '_') {
                for (start = pch++; isalnum(*pch) || *pch == '_'; pch++);
                char* text = copyn(buf, start, pch);
                if (strcmp(text, "int") == 0)
                    addTk(TYPE_INT);
                else if (strcmp(text, "var") == 0)
                    addTk(VAR);
                else if (strcmp(text, "function") == 0)
                    addTk(FUNCTION);
                else if (strcmp(text, "if") == 0)
                    addTk(IF);
                else if (strcmp(text, "else") == 0)
                    addTk(ELSE);
                else if (strcmp(text, "while") == 0)
                    addTk(WHILE);
                else if (strcmp(text, "return") == 0)
                    addTk(RETURN);
                else if (strcmp(text, "end") == 0)
                    addTk(END);
                else if (strcmp(text, "str") == 0)
                    addTk(TYPE_STR);
                else if (strcmp(text, "real") == 0)
                    addTk(TYPE_REAL);
                else {
                    tk = addTk(ID);
                    strcpy(tk->text, text);
                }
            }
            else if (isdigit(*pch)) {
                start = pch;
                while (isdigit(*pch)) pch++;

                if (*pch == '.') {
                    pch++;
                    if (!isdigit(*pch))
                        err("Malformed real number at line %d", line);

                    while (isdigit(*pch)) pch++;
                    char* text = copyn(buf, start, pch);
                    tk = addTk(REAL);
                    if (strlen(text) > MAX_STR)
                        err("Real number too long at line %d", line);
                    tk->r = atof(text);
                }
                else {
                    char* text = copyn(buf, start, pch);
                    tk = addTk(INT);
                    long value;
                    char* endptr;
                    errno = 0;
                    value = strtol(text, &endptr, 10);

                    if (errno == ERANGE || value > INT_MAX || value < INT_MIN) {
                        err("Integer number too long or too low at line %d", line);
                    }

                    tk->i = (int)value;
                }
            }
            else if (*pch == '.') {
                if (isdigit(pch[1])) {
                    start = pch - 1;
                    pch++;
                    while (isdigit(*pch)) pch++;

                    char* text = copyn(buf, start, pch);
                    tk = addTk(REAL);
                    if (strlen(text) > MAX_STR)
                        err("Real number too long at line %d", line);
                    tk->r = atof(text);
                }
                else {
                    err("Invalid character: '.' at line %d", line);
                    pch++;
                }
            }
            else {
                err("Invalid character: '%c' at line %d", *pch, line);
                pch++;
            }
            break;
        }
    }
}

const char* getTokenName(int code) {
    switch (code) {
    case ID: return "ID";
    case FUNCTION: return "FUNCTION";
    case IF: return "IF";
    case ELSE: return "ELSE";
    case WHILE: return "WHILE";
    case RETURN: return "RETURN";
    case VAR: return "VAR";
    case END: return "END";
    case TYPE_INT: return "TYPE_INT";
    case TYPE_REAL: return "TYPE_REAL";
    case TYPE_STR: return "TYPE_STR";
    case INT: return "INT";
    case REAL: return "REAL";
    case STR: return "STR";
    case ADD: return "ADD";
    case SUB: return "SUB";
    case MUL: return "MUL";
    case DIV: return "DIV";
    case ASSIGN: return "ASSIGN";
    case EQUAL: return "EQUAL";
    case NEQUAL: return "NOT_EQUAL";
    case LESS: return "LESS";
    case LESS_EQUAL: return "LESS_EQUAL";
    case GREATER: return "GREATER";
    case GREATER_EQUAL: return "GREATER_EQUAL";
    case GREATERQ: return "GREATERQ";
    case LPAR: return "LPAR";
    case RPAR: return "RPAR";
    case COMMA: return "COMMA";
    case COLON: return "COLON";
    case SEMICOLON: return "SEMICOLON";
    case FINISH: return "FINISH";
    case NOT: return "NOT";
    case AND: return "AND";
    case OR: return "OR";
    default: return "UNKNOWN";
    }
}

void showTokens() {
    for (int i = 0; i < nTokens; i++) {
        Token* tk = &tokens[i];
        const char* name = getTokenName(tk->code);
        if (strcmp(name, "UNKNOWN") == 0) {
            err("unknown token code: %d at line %d", tk->code, tk->line);
        }
        printf("%d %s", tk->line, name);
        if (tk->code == ID || tk->code == INT || tk->code == REAL || tk->code == STR) {
            if (tk->code == ID || tk->code == STR) {
                printf(":%s", tk->text);
            }
            else if (tk->code == INT) {
                printf(":%d", tk->i);
            }
            else if (tk->code == REAL) {
                printf(":%f", tk->r);
            }
        }
        printf("\n");
    }
}