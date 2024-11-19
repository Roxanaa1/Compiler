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
	
}