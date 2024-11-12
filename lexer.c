#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"
#include "utils.h"

Token tokens[MAX_TOKENS];
int nTokens;
int line = 1;

Token* addTk(int code) { 
	if (nTokens == MAX_TOKENS) err("too many tokens");
	Token* tk = &tokens[nTokens];
	tk->code = code;
	tk->line = line;
	nTokens++;
	return tk;
}

// copy in the dst buffer the string between [begin,end)
char* copyn(char* dst, const char* begin, const char* end) {
	char* p = dst;
	if (end - begin > MAX_STR) err("string too long");
	while (begin != end) *p++ = *begin++;
	*p = '\0';
	return dst;
}
const char* tkCodeName(int code) {
	switch (code) {
	case ID: return "ID";
	case INT: return "INT";
	case REAL: return "REAL";
	case STR: return "STR";
	case VAR: return "VAR";
	case FUNCTION: return "FUNCTION";
	case IF: return "IF";
	case ELSE: return "ELSE";
	case WHILE: return "WHILE";
	case END: return "END";
	case RETURN: return "RETURN";
	case TYPE_INT: return "TYPE_INT";
	case TYPE_REAL: return "TYPE_REAL";
	case TYPE_STR: return "TYPE_STR";
	case ADD: return "ADD";
	case SUB: return "SUB";
	case MUL: return "MUL";
	case DIV: return "DIV";
	case AND: return "AND";
	case OR: return "OR";
	case NOT: return "NOT";
	case ASSIGN: return "ASSIGN";
	case EQUAL: return "EQUAL";
	case NOTEQ: return "NOTEQ";
	case LESS: return "LESS";
	case LESSEQ: return "LESSEQ";
	case GREATER: return "GREATER";
	case GREATEREQ: return "GREATEREQ";
	case COMMA: return "COMMA";
	case COLON: return "COLON";
	case SEMICOLON: return "SEMICOLON";
	case LPAR: return "LPAR";
	case RPAR: return "RPAR";
	case FINISH: return "FINISH";
	default: return "UNKNOWN";
	}
}
void tokenize(const char* pch) {
	const char* start;
	Token* tk;
	char buf[MAX_STR + 1];
	for (;;) {
		switch (*pch) {
		case ' ':case '\t':
			pch++;
			break;
		case '\r':// Windows: \r\n, Linux: \n, MacOS, OS X: \r or \n)
			if (pch[1] == '\n')
				pch++;
		case '\n':
			line++;
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
		case '&':
			if (pch[1] == '&') {
				addTk(AND);
				pch += 2;
			}
			else {
				err("Nu este caracter valid ,trebuia && sau &: %c (%d)", *pch, *pch);
			}
			break;
		case '|':
			if (pch[1] == '|') {
				addTk(OR);
				pch += 2;
			}
			else {
				err("Nu este caracter valid,trebuia | sau ||: %c (%d)", *pch, *pch);
			}
			break;
		case '!':
			if (pch[1] == '=') {
				addTk(NOTEQ);
				pch += 2;
			}
			else {
				addTk(NOT);
				pch++;
			}
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
		case '<':
			if (pch[1] == '=') {
				addTk(LESSEQ);
				pch += 2;
			}
			else {
				addTk(LESS);
				pch++;
			}
			break;
		case '>':
			if (pch[1] == '=') {
				addTk(GREATEREQ);
				pch += 2;
			}
			else {
				addTk(GREATER);
				pch++;
			}
			break;
		case '#':
			while (*pch != '\n' && *pch != '\0') pch++;
			break;
		case '"':
			pch++;
			start = pch;
			while (*pch != '"' && *pch != '\0') pch++;
			if (*pch == '"') {
				char* text = copyn(buf, start, pch);
				tk = addTk(STR);
				strcpy(tk->text, text);
				pch++;
			}
			else {
				err("String-ul nu a fost definit corect");
			}
			break;
			//INT REAL si ID
		default:
			if (isdigit(*pch)) {
				for (start = pch; isdigit(*pch); pch++);
				if (*pch == '.') {
					pch++;
					if (isdigit(*pch)) {
						while (isdigit(*pch)) pch++;
						char* text = copyn(buf, start, pch);
						tk = addTk(REAL);
						tk->r = atof(text);
					}
					else {
						err("Formatul numarului real este invalid");
					}
				}
				else {
					char* text = copyn(buf, start, pch);
					tk = addTk(INT);
					tk->i = atoi(text);
				}
			}
			else if (isalpha(*pch) || *pch == '_') { // Identificatori
				for (start = pch++; isalnum(*pch) || *pch == '_'; pch++);
				char* text = copyn(buf, start, pch);
				if (strcmp(text, "int") == 0)
					addTk(TYPE_INT);
				else if (strcmp(text, "real") == 0)
					addTk(TYPE_REAL);
				else if (strcmp(text, "str") == 0)
					addTk(TYPE_STR);
				else if (strcmp(text, "function") == 0)
					addTk(FUNCTION);
				else if (strcmp(text, "if") == 0)
					addTk(IF);
				else if (strcmp(text, "else") == 0)
					addTk(ELSE);
				else if (strcmp(text, "return") == 0)
					addTk(RETURN);
				else if (strcmp(text, "var") == 0)
					addTk(VAR);
				else if (strcmp(text, "while") == 0)
					addTk(WHILE);
				else if (strcmp(text, "end") == 0)
					addTk(END);
				else {
					tk = addTk(ID);
					strcpy_s(tk->text, sizeof(tk->text), text);
				}
			}
			else {
				err("Invalid char: %c (%d)", *pch, *pch);
			}
		}
	}
}

void showTokens() {
	for (int i = 0; i < nTokens; i++) {
		Token* tk = &tokens[i];
		printf("%d ", tk->line);  

		switch (tk->code) {
		case ID:
			printf("ID:%s\n", tk->text);
			break;
		case INT:
			printf("INT:%d\n", tk->i);
			break;
		case REAL:
			printf("REAL:%.6f\n", tk->r);
			break;
		case STR:
			printf("STR:%s\n", tk->text);
			break;
		case VAR:
			printf("VAR\n");
			break;
		case FUNCTION:
			printf("FUNCTION\n");
			break;
		case IF:
			printf("IF\n");
			break;
		case ELSE:
			printf("ELSE\n");
			break;
		case WHILE:
			printf("WHILE\n");
			break;
		case END:
			printf("END\n");
			break;
		case RETURN:
			printf("RETURN\n");
			break;
		case TYPE_INT:
			printf("TYPE_INT\n");
			break;
		case TYPE_REAL:
			printf("TYPE_REAL\n");
			break;
		case TYPE_STR:
			printf("TYPE_STR\n");
			break;
		case ADD:
			printf("ADD\n");
			break;
		case SUB:
			printf("SUB\n");
			break;
		case MUL:
			printf("MUL\n");
			break;
		case DIV:
			printf("DIV\n");
			break;
		case AND:
			printf("AND\n");
			break;
		case OR:
			printf("OR\n");
			break;
		case NOT:
			printf("NOT\n");
			break;
		case ASSIGN:
			printf("ASSIGN\n");
			break;
		case EQUAL:
			printf("EQUAL\n");
			break;
		case NOTEQ:
			printf("NOTEQ\n");
			break;
		case LESS:
			printf("LESS\n");
			break;
		case LESSEQ:
			printf("LESSEQ\n");
			break;
		case GREATER:
			printf("GREATER\n");
			break;
		case GREATEREQ:
			printf("GREATEREQ\n");
			break;
		case COMMA:
			printf("COMMA\n");
			break;
		case COLON:
			printf("COLON\n");
			break;
		case SEMICOLON:
			printf("SEMICOLON\n");
			break;
		case LPAR:
			printf("LPAR\n");
			break;
		case RPAR:
			printf("RPAR\n");
			break;
		case FINISH:
			printf("FINISH\n");
			break;
		default:
			printf("Unknown token: %d\n", tk->code);
			break;
		}
	}
}




