#pragma once
#include<stdbool.h>
const char* tkCodeName(int code);
bool defVar();
bool defFunc();
bool block();
bool baseType();
bool funcParams();
bool funcParam();
bool instr();
bool expr();
bool exprLogic();
bool exprAssign();
bool exprComp();
bool exprAdd();
bool exprMul();
bool exprPrefix();
bool factor();
void parse();
