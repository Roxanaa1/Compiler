#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "parser.h"
#include "lexer.h"

int iTk;                                                                                             // iterator în tokens
Token* consumed;     
bool isFunctionContext = false; 
_Noreturn; void tkerr(const char* fmt, ...) {
    fprintf(stderr, "Eroare la linia %d: ", tokens[iTk].line);
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

bool consume(int code) {
    printf("consume(%s)", tkCodeName(code));
    if (tokens[iTk].code == code) {
        consumed = &tokens[iTk++];                                                                  //consumed reține un pointer către ultimul token consumat.
        printf(" => consumed\n");
        return true;
    }
    printf(" => found %s\n", tkCodeName(tokens[iTk].code));
    return false;
}

// program ::= ( defVar | defFunc | block )* FINISH
bool program() {
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
        tkerr("Eroare de sintaxa, lipseste 'FINISH'");
    }
    return false;
}

// defVar ::= VAR ID COLON baseType SEMICOLON
bool defVar() {
    int start = iTk;
    if (consume(VAR)) {
        if (consume(ID)) {
            if (consume(COLON)) {
                if (baseType()) {
                    if (consume(SEMICOLON)) {
                        return true;
                    }
                    else {
                        tkerr("Lipseste ';' la sfarsitul declaratiei de variabila");
                    }
                }
            }
            else {
                tkerr("Lipseste ':' dupa identificatorul variabilei");
            }
        }
        else {
            tkerr("Lipseste identificatorul dupa 'VAR'");
        }
    }
    iTk = start;
    return false;
}

// defFunc ::= FUNCTION ID LPAR funcParams? RPAR COLON baseType defVar* block END
bool defFunc() {
    int start = iTk;
    if (consume(FUNCTION)) {
        if (consume(ID)) {
            if (consume(LPAR)) {
                
                if (funcParams()) {}

                if (consume(RPAR)) {
                   
                    if (consume(COLON)) {
                       
                        isFunctionContext = true;
                        if (baseType()) {
                            
                            while (defVar()) {}
                            if (block()) {
                                if (consume(END)) {
                                    return true;
                                }
                                else {
                                    tkerr("Lipseste 'END' la sfarsitul functiei");
                                }
                            }
                        }
                        else {
                           
                            tkerr("Lipseste tipul functiei");
                        }
                    }
                    else {
                        tkerr("Lipseste ':' dupa parametrii functiei");
                    }
                }
                else {
                    tkerr("Lipseste ')' dupa parametrii functiei");
                }
            }
            else {
                tkerr("Lipseste '(' dupa 'FUNCTION'");
            }
        }
        else {
            tkerr("Lipseste identificatorul dupa 'FUNCTION'");
        }
    }
    iTk = start;
    return false;
}


// block ::= instr+
bool block() {
    int start = iTk;
    if (instr()) {               
        while (instr()) {}
        return true;
    }
    iTk = start;
    return false;
}

// baseType ::= TYPE_INT | TYPE_REAL | TYPE_STR
bool baseType() {
    if (consume(TYPE_INT)) return true;
    if (consume(TYPE_REAL)) return true;
    if (consume(TYPE_STR)) return true;

    if (isFunctionContext) {
        tkerr("Lipseste tipul functiei");
    }
    else {
        tkerr("Lipseste tipul parametrului");
    }
    return false;
}


// funcParams ::= funcParam ( COMMA funcParam )*
bool funcParams() {
   
    if (!funcParam()) {
        return false;  
    }

    while (consume(COMMA)) { 
        if (consume(COMMA)) { 
            tkerr("Lipseste parametru dupa ','"); 
            return false;
        }

        if (!funcParam()) {
            tkerr("Lipseste parametru dupa ','");  
            return false;
        }
    }

    return true;
}
bool funcParam() {
    int start = iTk; 
    if (consume(ID)) { 
        if (consume(COLON)) { 
            if (baseType()) { 
                return true; 
            }
            else {
                tkerr("Lipseste tipul parametrului");
            }
        }
        else {
            tkerr("Lipseste ':' dupa identificatorul parametrului"); 
        }
    }
    else {
        tkerr("Lipseste identificatorul parametrului inainte/dupa virgula"); 
    }
    iTk = start; 
    return false; 
}
// instr ::= expr? SEMICOLON
//           | IF LPAR expr RPAR block ( ELSE block )? END
//           | RETURN expr SEMICOLON
//           | WHILE LPAR expr RPAR block END
bool instr() {
    int start = iTk;
    if (expr()) {
        if (consume(SEMICOLON)) return true;
        else tkerr("Lipseste ';' dupa expresie");
    }
    else if (consume(IF)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (block()) {
                        if (consume(ELSE)) {
                            if (!block()) {
                                tkerr("Lipseste blocul dupa 'else'"); // Aici am adaugat mesajul de eroare pentru block lipsa dupa else
                                iTk = start;
                                return false;
                            }
                        }
                        if (consume(END)) return true;
                        else tkerr("Lipseste 'END' la sfarsitul blocului 'if'");
                    }
                }
                else tkerr("Lipseste ')' dupa conditia 'if'");
            }
            else tkerr("Conditie invalida pentru 'if'");
        }
        else tkerr("Lipseste '(' dupa 'if'");
    }
    else if (consume(RETURN)) {
        if (expr()) {
            if (consume(SEMICOLON)) return true;
            else tkerr("Lipseste ';' dupa expresia 'RETURN'");
        }
        else tkerr("Expresie invalida dupa 'RETURN'");
    }
    else if (consume(WHILE)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (block()) {
                        if (consume(END)) return true;
                        else tkerr("Lipseste 'END' la sfarsitul blocului 'while'");
                    }
                }
                else tkerr("Lipseste ')' dupa conditia 'while'");
            }
            else tkerr("Conditie invalida pentru 'while'");
        }
        else tkerr("Lipseste '(' dupa 'while'");
    }
    iTk = start;
    return false;
}

// expr ::= exprLogic
bool expr() {
    return exprLogic();
}

// exprLogic ::= exprAssign ( ( AND | OR ) exprAssign )*
bool exprLogic() {
    int start = iTk;
    if (exprAssign()) {
        while (consume(AND) || consume(OR)) {
            if (!exprAssign()) {
                tkerr("Expresie invalida dupa operatorul logic");
                iTk = start;
                return false;
            }
        }
        return true;
    }
    iTk = start;
    return false;
}

// exprAssign ::= ( ID ASSIGN )? exprComp
bool exprAssign() {
    int start = iTk;
    if (consume(ID)) {
        if (consume(ASSIGN)) {
            if (exprComp()) return true;
            tkerr("Expresie invalida dupa '='");
            iTk = start;
            return false;
        }
    }
    iTk = start;
    return exprComp();
}

// exprComp ::= exprAdd ( ( LESS | EQUAL ) exprAdd )?
bool exprComp() {
    int start = iTk;
    if (exprAdd()) {
        if (consume(LESS) || consume(EQUAL)) {
            if (exprAdd()) return true;
            tkerr("Expresie invalida dupa operatorul de comparatie");
            iTk = start;
            return false;
        }
        return true;
    }
    iTk = start;
    return false;
}

// exprAdd ::= exprMul ( ( ADD | SUB ) exprMul )*
bool exprAdd() {
    int start = iTk;
    if (exprMul()) {
        while (consume(ADD) || consume(SUB)) {
            if (!exprMul()) {
                tkerr("Expresie invalida dupa operatorul de adunare/scadere");
                iTk = start;
                return false;
            }
        }
        return true;
    }
    iTk = start;
    return false;
}

// exprMul ::= exprPrefix ( ( MUL | DIV ) exprPrefix )*
bool exprMul() {
    int start = iTk;
    if (exprPrefix()) {
        while (consume(MUL) || consume(DIV)) {
            if (!exprPrefix()) {
                tkerr("Expresie invalida dupa operatorul de inmultire/impartire");
                iTk = start;
                return false;
            }
        }
        return true;
    }
    iTk = start;
    return false;
}

// exprPrefix ::= ( SUB | NOT )? factor
bool exprPrefix() {
    if (consume(SUB) || consume(NOT)) {
        if (!factor()) {
            tkerr("Factor invalid dupa operatorul de prefix");
            return false;
        }
        return true;
    }
    return factor();
}

// factor ::= INT | REAL | STR | LPAR expr RPAR | ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
bool factor() {
    int start = iTk;
    if (consume(INT) || consume(REAL) || consume(STR)) return true;

    if (consume(LPAR)) {
        if (expr()) {
            if (consume(RPAR)) return true;
            tkerr("Lipseste ')' dupa expresie");
        }
        else tkerr("Expresie invalida intre paranteze");
    }
    else if (consume(ID)) {
        if (consume(LPAR)) {
            if (expr()) {
                while (consume(COMMA)) {
                    if (!expr()) {
                        tkerr("Expresie invalida după ',' în apelul functiei");
                        iTk = start;
                        return false;
                    }
                }
            }
            if (consume(RPAR)) return true;
            tkerr("Lipseste ')' la sfarsitul apelului de functie");
        }
        return true;
    }
    iTk = start;
    return false;
}

void parse() {                                                                                                   //initializeaza proecsul de parsare
    iTk = 0;
    program();
}