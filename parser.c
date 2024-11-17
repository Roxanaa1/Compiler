#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "lexer.h"

int iTk;	// the iterator in tokens
Token* consumed;	// the last consumed token
extern Token tokens[];

// same as err, but also prints the line of the current token
_Noreturn void tkerr(const char* fmt, ...) {
    fprintf(stderr, "error in line %d: ", tokens[iTk].line - 1);
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

bool consume(int code) {
    printf("consume(%s)", getTokenName(code));
    if (tokens[iTk].code == code) {
        consumed = &tokens[iTk++];
        printf(" => consumed: %s\n", getTokenName(consumed->code));
        return true;
    }
    printf(" => found %s\n", getTokenName(tokens[iTk].code));
    return false;
}

bool program();
bool defVar();
bool defFunc();
bool funcParam();
bool funcParams();
bool baseType();
bool block();
bool instr();
bool expr();
bool exprLogic();
bool exprAssign();
bool exprComp();
bool exprAdd();
bool exprMul();
bool exprPrefix();
bool factor();

bool factor() {
    printf("factor() called at token index %d\n", iTk);
    int start = iTk;

    if (consume(INT) || consume(REAL) || consume(STR)) {
        return true;
    }

    if (consume(LPAR)) {
        if (expr()) {
            if (consume(RPAR)) {
                return true;
            }
            tkerr("Eroare de sintaxa: Lipseste ')' pentru a inchide expresia");
        }
        tkerr("Eroare de sintaxa: Expresie invalida dupa '('");
    }

    if (consume(ID)) {
        if (consume(LPAR)) {
            if (!expr()) {
                if (!consume(RPAR)) {
                    tkerr("Eroare de sintaxa: se asteapta o expresie sau ')'");
                }
            }
            else {
                while (consume(COMMA)) {
                    if (!expr()) {
                        tkerr("Eroare de sintaxa: se asteapta o expresie dupa ','");
                    }
                }
                if (!consume(RPAR)) {
                    tkerr("Eroare de sintaxa: lipseste ')' dupa lista de argumente");
                }
            }
            return true;
        }
        return true;
    }

    iTk = start;
    return false;
}

bool exprPrefix() {
    printf("exprPrefix() called at token index %d\n", iTk);
    int start = iTk;
    while (consume(SUB) || consume(NOT));
    if (factor()) {
        return true;
    }
    iTk = start;
    return false;
}

bool exprMul() {
    printf("exprMul() called at token index %d\n", iTk);
    int start = iTk;
    if (exprPrefix()) {
        while (consume(MUL) || consume(DIV)) {
            if (!exprPrefix()) {
                tkerr("Operand lipsa dupa operatorul '*' sau '/'");
            }
        }
        return true;
    }
    iTk = start;
    return false;
}

bool exprAdd() {
    printf("exprAdd() called at token index %d\n", iTk);
    int start = iTk;
    if (exprMul()) {
        while (consume(ADD) || consume(SUB)) {
            if (!exprMul()) {
                tkerr("Eroare de sintaxa: Operand lipsa dupa operatorul '+' sau '-'");
            }
        }
        return true;
    }
    iTk = start;
    return false;
}

bool exprComp() {
    printf("exprComp() called at token index %d\n", iTk);
    int start = iTk;
    if (exprAdd()) {
        if (consume(LESS) || consume(LESS_EQUAL) || consume(EQUAL) || consume(GREATER) || consume(GREATER_EQUAL) || consume(NEQUAL)) {
            if (!exprAdd()) {
                tkerr("Eroare de sintaxa: Operand lipsa dupa operatorul de comparatie");
            }
        }
        return true;
    }
    iTk = start;
    return false;
}

bool exprAssign() {
    printf("exprAssign() called at token index %d\n", iTk);
    int start = iTk;
    if (consume(ID)) {
        if (consume(ASSIGN)) {
            if (exprComp()) {
                return true;
            }
            tkerr("Eroare de sintaxa: Expresie lipsa dupa operatorul '='");
        }
        iTk = start;
    }
    return exprComp();
}

bool exprLogic() {
    printf("exprLogic() called at token index %d\n", iTk);
    int start = iTk;
    if (exprAssign()) {
        while (consume(AND) || consume(OR)) {
            if (!exprAssign()) {
                tkerr("Eroare de sintaxa: Expresie invalida dupa operatorul logic '&&' sau '||'");
            }
        }
        return true;
    }
    iTk = start;
    return false;
}

bool expr() {
    printf("expr() called at token index %d\n", iTk);
    return exprLogic();
}

bool instr() {
    printf("instr() called at token index %d\n", iTk);
    int start = iTk;

    if (consume(IF)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (block()) {
                        if (consume(ELSE)) {
                            if (!block()) {
                                tkerr("Lipseste bloc dupa ELSE");
                            }
                        }
                        if (consume(END)) {
                            return true;
                        }
                        tkerr("Eroare de sintaxa: lipseste END dupa blocul IF");
                    }
                    tkerr("Eroare de sintaxa: lipseste bloc fupa IF");
                }
                tkerr("Eroare de sintaxa: lipseste ')' dupa conditia din IF");
            }
            tkerr("Eroare: lipseste expresie in conditia IF");
        }
        tkerr("Eroare de sintaxa: lipseste '(' dupa IF");
    }

    iTk = start;
    if (consume(RETURN)) {
        if (expr()) {
            if (consume(SEMICOLON)) {
                return true;
            }
            tkerr("Eroare de sintaxa: Lipseste ';' dupa return");
        }
        tkerr("Eroare: Expresie lipsa dupa return");
    }

    iTk = start;
    if (consume(WHILE)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (block()) {
                        if (consume(END)) {
                            return true;
                        }
                        tkerr("Eroare de sintaxa: Lipseste END dupa blocul WHILE");
                    }
                    tkerr("Eroare de sintaxa: Bloc lipsa dupa WHILE");
                }
                tkerr("Eroare de sintaxa: Lipseste ')' dupa expresia din WHILE");
            }
            tkerr("Eroare de sintaxa: Expresie lipsa in conditia WHILE");
        }
        tkerr("Eroare de sintaxa: Lipseste '(' dupa WHILE");
    }
    iTk = start;
    if (expr()) {
        if (consume(SEMICOLON)) {
            return true;
        }
        tkerr("Eroare de sintaxa: lipseste ';' dupa expresie");
    }

    iTk = start;
    return false;
}

bool block() {
    printf("block() called at token index %d\n", iTk);
    int start = iTk;
    if (instr()) {
        while (instr());
        return true;
    }
    iTk = start;
    return false;
}

bool defFunc() {
    printf("defFunc() called at token index %d\n", iTk);
    int start = iTk;
    if (consume(FUNCTION)) {
        if (consume(ID)) {
            if (consume(LPAR)) {
                funcParams();
                if (consume(RPAR)) {
                    if (consume(COLON)) {
                        if (baseType()) {
                            while (defVar());
                            if (block()) {
                                if (consume(END)) {
                                    return true;
                                }
                                tkerr("Eroare de sintaxa: Lipseste END dupa definitia functiei");
                            }
                            tkerr("Eroare de sintaxa: Bloc lipsa in definirea functiei");
                        }
                        tkerr("Eroare de sintaxa: Tip lipsa in definirea functiei");
                    }
                    tkerr("Eroare de sintaxa: Lipseste ':' dupa parametrii functiei");
                }
                tkerr("Eroare de sintaxa: Lipseste ')' dupa lista de parametrii");
            }
            tkerr("Eroare de sintaxa: Lipseste '(' dupa identificatorul functiei");
        }
        tkerr("Eroare de sintaxa: Identificator lipsa pentru functie");
    }
    iTk = start;
    return false;
}

bool funcParam() {
    printf("funcParam() called at token index %d\n", iTk);
    int start = iTk;
    if (consume(ID)) {
        if (consume(COLON)) {
            if (baseType()) {
                return true;
            }
            tkerr("Eroare de sintaxa: Lipseste tipul de baza pentru parametrul functiei");
        }
        tkerr("Eroare de sintaxa: Lipseste ':' dupa identificatorul parametrului");
    }
    iTk = start;
    return false;
}

bool funcParams() {
    printf("funcParams() called at token index %d\n", iTk);
    int start = iTk;
    if (funcParam()) {
        while (consume(COMMA)) {
            if (!funcParam()) {
                tkerr("Eroare de sintaxa: Parametru lipsa dupa virgula in lista de parametrii");
            }
        }
        return true;
    }
    iTk = start;
    return false;
}

bool baseType() {
    printf("baseType() called at token index %d\n", iTk);
    int start = iTk;
    if (consume(TYPE_INT) || consume(TYPE_REAL) || consume(TYPE_STR)) {
        return true;
    }
    iTk = start;
    tkerr("Eroare de sintaxa: Tip de baza invalid, asteptat: int, real sau str");
    return false;
}

bool defVar() {
    printf("defVar() called at token index %d\n", iTk);
    int start = iTk;
    if (consume(VAR)) {
        if (consume(ID)) {
            if (consume(COLON)) {
                if (baseType()) {
                    if (consume(SEMICOLON)) {
                        return true;
                    }
                    tkerr("Eroare de sintaxa: Lipseste ';' dupa definitia variabilei");
                }
                tkerr("Eroare de sintaxa: Lipseste tip de baza pentru variabila");
            }
            tkerr("Eroare de sintaxa: Lipseste ':' dupa identificatorul variabilei");
        }
        tkerr("Eroare de sintaxa: Identificator lipsa al variabilei");
    }
    iTk = start;
    return false;
}


// program ::= ( defVar | defFunc | block )* FINISH
bool program() {
    int start = iTk;
    for (;;) {
        if (defVar()) {}
        else if (defFunc()) {}
        else if (block()) {}
        else break;
    }
    if (consume(FINISH)) {
        return true;
    }
    else {
        iTk = start;
        tkerr("syntax error: Program incomplet sau incorect");
    }
    return false;
}

void parse() {
    iTk = 0;
    if (!program()) {
        tkerr("Parser failed");
    }
}
