#include<stdlib.h>
#include<stdio.h>
#include "parser.h"
//#include "lexer.h"
char* loadFile(const char* filename);
void tokenize(const char* inbuf);
void showTokens(void);
int main()
{
	char* inbuf = loadFile("1.q");
	tokenize(inbuf);
	showTokens();
	parse(); 
	free(inbuf); 
	return 0;
	//trebuie sa adaug apelul lui parse 
	//trebuie sa adaug parse.c si parse.h
	//avem reguli de testare :
	// la inceputul fiecare functii ar trebui put 
	//modificarea functiei consume a.i. sa vedem ce am consumat 
	//avem in laborator functia consume care ne ajuta pentru depanare 
}