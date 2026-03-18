#include "needs.h"

int curr = 0;
int wrd = 0;
int currwrd = 0;
int SbrCount = 0;
int labelsCount = 0;
int currReg = 3;
int totalSbr = 0;
int csbr = 0;
int cinst = 0;
char *tr = NULL;

char input[] = {
    "( $start \n"
    "( $r 1 \n"
    "X0 <- #0 \n"
    "X2 <- #0 \n"
    ":loop CB X0 == X1 :evn \n"
    "CB X0 > X1 :odd \n"
    "X0 +<- #2 \n"
    "b :loop \n"
    ":odd X2 <- #0 \n"
    ":evn X2 <- #1 \n"
    "return \n"
    ") \n"
    "( $start 3 \n"
    "X1 <- #6 \n"
    "bl $r \n"
    ") \n"
    ")"
};

char s[50];
char word[50];
TA1STATE t;
Names labels[10];
Names Sbr[10];
TSbr tsbr[20];

int main(void) {
    initLabels();
    initVars();
    verify(&t, input);
    translate(tsbr);
    printf("%s\n", tr);
    free(tr);
    return 0;
}

wordType classifyWord(char *word) {
    if (strncmp(word, W, 1) == 0 && isdigit(word[1])) {
        int num = 0, i = 1;
        while (isdigit(word[i])) { num = num * 10 + (word[i] - '0'); i++; }
        if (word[i] == Null && num >= 0 && num < 29) return R32;
    } else if (strncmp(word, X, 1) == 0 && isdigit(word[1])) {
        int num = 0, i = 1;
        while (isdigit(word[i])) { num = num * 10 + (word[i] - '0'); i++; }
        if (word[i] == Null && num >= 0 && num < 29) return R64;
    } else if (strcmp(word, "return") == 0) return RET;
    else if (strcmp(word, "CB")  == 0) return CB;
    else if (strcmp(word, "bl")  == 0) return bl;
    else if (strcmp(word, "<")   == 0) return LT;
    else if (strcmp(word, ">")   == 0) return GT;
    else if (strcmp(word, "==")  == 0) return EQ;
    else if (strcmp(word, "!=")  == 0) return NE;
    else if (strcmp(word, "<=")  == 0) return LE;
    else if (strcmp(word, ">=")  == 0) return GE;
    else if (strcmp(word, "mem") == 0) return MEM;
    else if (strcmp(word, br)    == 0) return BR;
    else if (strcmp(word, "fp")  == 0) return FP;
    else if (strcmp(word, "sp")  == 0) return SP;
    else if (strcmp(word, nwline)== 0) return Newline;
    else if (strcmp(word, plus)  == 0) return Plus;
    else if (strcmp(word, parrow)== 0) return PArrow;
    else if (strcmp(word, minus) == 0) return Minus;
    else if (strcmp(word, sarrow)== 0) return SArrow;
    else if (strcmp(word, mul)   == 0) return Mul;
    else if (strcmp(word, marrow)== 0) return MArrow;
    else if (strcmp(word, ls)    == 0) return LS;
    else if (strcmp(word, lsarrow)==0) return LArrow;
    else if (strcmp(word, rsarrow)==0) return RArrow;
    else if (strcmp(word, rs)    == 0) return RS;
    else if (strcmp(word, arrow) == 0) return Arrow;
    else if (strcmp(word, open)  == 0) return Oparen;
    else if (strcmp(word, close) == 0) return Cparen;
    else if (strncmp(word, "#", 1) == 0) {
        int num = 0, i = 1;
        while (word[i] != Null) {
            if (!isdigit(word[i])) return Unknown;
            num = num * 10 + (word[i] - '0');
            i++;
        }
        return (num % 8 == 0) ? Div8 : Imm;
    } else if (strncmp(word, ":", 1) == 0) {
        int i = 1;
        if (!isalpha(word[i])) { printf("Invalid Label\n"); exit(0); }
        i++;
        while (word[i] != Null) {
            if (!isalnum(word[i])) { printf("Invalid Label\n"); exit(0); }
            i++;
        }
        return LabelTok;
    } else if (strncmp(word, doller, 1) == 0) {
        int i = 1;
        if (!isalpha(word[i])) { printf("Invalid Sbr Name\n"); exit(0); }
        i++;
        while (word[i] != Null) {
            if (!isalnum(word[i])) { printf("Invalid Sbr Name\n"); exit(0); }
            i++;
        }
        return Sbrname;
    } else if (isdigit(word[0])) {
        int i = 1;
        while (word[i] != Null && isdigit(word[i])) i++;
        if (word[i] == Null) return Prmtr;
    }
    printf("Invalid token: '%s'\n", word);
    return Unknown;
}

void nextword(TA1STATE *t, char *input) {
    int i = 0;
    int len = (int)strlen(input);
    while (curr < len && (input[curr] == Space || input[curr] == Tab))
        curr++;
    if (curr >= len || input[curr] == Null) {
        t->nw[0] = Null;
        t->currentType = EOT;
        return;
    }
    currwrd = curr;
    if (input[curr] == '\n') {
        t->nw[0] = '\n'; t->nw[1] = Null;
        curr++; wrd++;
        t->currentType = Newline;
        return;
    }
    while (curr < len && input[curr] != Space && input[curr] != Tab
           && input[curr] != '\n' && input[curr] != Null) {
        t->nw[i++] = input[curr++];
    }
    t->nw[i] = Null;
    wrd++;
    t->currentType = classifyWord(t->nw);
}

void getWord(int index) {
    int i = 0;
    while (input[index] != Null && input[index] != Space && input[index] != Tab) {
        word[i++] = input[index++];
    }
    word[i] = Null;
}

void printSbrNames(void) {
    printf("Subroutine Names:\n");
    for (int i = 0; i < SbrCount; i++)
        if (Sbr[i].name) printf("  Sbr[%d]: %s\n", i, Sbr[i].name);
}

void printLabelNames(void) {
    printf("Label Names:\n");
    for (int i = 0; i < labelsCount; i++)
        if (labels[i].name) printf("  Label[%d]: %s\n", i, labels[i].name);
}

void is_set(Names *nm, int count) {
    for (int i = 0; i < count; i++) {
        if (nm[i].isSet == 0) {
            printf("Error: label '%s' referenced but never defined\n", nm[i].name);
            exit(1);
        }
    }
}

int validateRet(TA1STATE *t) { return check(t, RET); }

int check(TA1STATE *t, wordType w) { return t->currentType == w; }

int checkCO(TA1STATE *t) {
    if      (check(t, EQ)) { tsbr[csbr].instr[cinst].type = BEQ; return 1; }
    else if (check(t, NE)) { tsbr[csbr].instr[cinst].type = BNE; return 1; }
    else if (check(t, LT)) { tsbr[csbr].instr[cinst].type = BLT; return 1; }
    else if (check(t, GT)) { tsbr[csbr].instr[cinst].type = BGT; return 1; }
    else if (check(t, LE)) { tsbr[csbr].instr[cinst].type = BLE; return 1; }
    else if (check(t, GE)) { tsbr[csbr].instr[cinst].type = BGE; return 1; }
    return 0;
}

int V32(TA1STATE *t) { return check(t, R32) || check(t, Imm); }
int V64(TA1STATE *t) { return check(t, R64) || check(t, Imm) || check(t, SP); }

int OP(TA1STATE *t) {
    if      (check(t, PArrow)) tsbr[csbr].instr[cinst].type = ADD;
    else if (check(t, SArrow)) tsbr[csbr].instr[cinst].type = SUB;
    else if (check(t, MArrow)) tsbr[csbr].instr[cinst].type = MUL;
    else if (check(t, LArrow)) tsbr[csbr].instr[cinst].type = LSL;
    else if (check(t, RArrow)) tsbr[csbr].instr[cinst].type = LSR;
    else return 0;
    return 1;
}

int BO(TA1STATE *t) {
    if      (check(t, Plus))  tsbr[csbr].instr[cinst].type = ADD;
    else if (check(t, Minus)) tsbr[csbr].instr[cinst].type = SUB;
    else if (check(t, Mul))   tsbr[csbr].instr[cinst].type = MUL;
    else if (check(t, LS))    tsbr[csbr].instr[cinst].type = LSL;
    else if (check(t, RS))    tsbr[csbr].instr[cinst].type = LSR;
    else return 0;
    return 1;
}

void validate(TA1STATE *t, wordType w) {
    if (t->currentType != w) {
        printf("Syntax error: unexpected token '%s'\n", t->nw);
        exit(1);
    }
}

int isExist(Names *arr, TA1STATE *t) {
    for (int i = 0; i < 10; i++)
        if (arr[i].name && strcmp(arr[i].name, t->nw) == 0) return i;
    return -1;
}

void store(Names *arr, TA1STATE *t, int count, int isset) {
    if (count >= 10) { printf("Error: exceeded maximum label/subroutine count\n"); exit(1); }
    arr[count].name = malloc(strlen(t->nw) + 1);
    if (!arr[count].name) { printf("malloc failed\n"); exit(1); }
    strcpy(arr[count].name, t->nw);
    arr[count].isSet = isset;
}

int validateBrSbrlbl(TA1STATE *t) {
    if (check(t, LabelTok)) {
        tsbr[csbr].instr[cinst].lbl = currwrd;
        tsbr[csbr].instr[cinst].sbr = -1;
        int idx = isExist(labels, t);
        if (idx < 0) { store(labels, t, labelsCount, 0); labelsCount++; }
        return 1;
    } else if (check(t, Sbrname)) {
        tsbr[csbr].instr[cinst].sbr = currwrd;
        tsbr[csbr].instr[cinst].lbl = -1;
        int idx = isExist(Sbr, t);
        if (idx < 0) { store(Sbr, t, SbrCount, 0); SbrCount++; }
        return 1;
    }
    return 0;
}

int verifyCB(TA1STATE *t, char *input) {
    if (!check(t, CB)) return 0;
    nextword(t, input);
    tsbr[csbr].instr[cinst].op1 = currwrd;
    int is64 = check(t, R64);
    int is32 = check(t, R32);
    if (!is64 && !is32) return 0;
    nextword(t, input);
    if (!checkCO(t)) return 0;
    nextword(t, input);
    int op2ok = is64 ? (check(t, R64) || check(t, Imm)) : (check(t, R32) || check(t, Imm));
    if (!op2ok) return 0;
    tsbr[csbr].instr[cinst].op2 = currwrd;
    nextword(t, input);
    if (!validateBrSbrlbl(t)) return 0;
    cinst++;
    return 1;
}

int A(TA1STATE *t, char *input) {
    if (!check(t, R32) && !check(t, R64)) return 0;
    tsbr[csbr].instr[cinst].dest = currwrd;
    tsbr[csbr].instr[cinst].op1  = currwrd;
    currReg = check(t, R64) ? 1 : 0;
    nextword(t, input);
    if (currReg == 1) {
        if (OP(t)) {
            nextword(t, input);
            if (V64(t)) {
                tsbr[csbr].instr[cinst].op2 = currwrd;
                cinst++;
                return 1;
            }
            exit(1);
        } else if (check(t, Arrow)) {
            nextword(t, input);
            tsbr[csbr].instr[cinst].op1 = currwrd;
            if (check(t, R64)) {
                nextword(t, input);
                if (BO(t)) {
                    nextword(t, input);
                    if (V64(t)) {
                        tsbr[csbr].instr[cinst].op2 = currwrd;
                        cinst++;
                        return 1;
                    }
                    exit(1);
                } else if (check(t, Newline)) {
                    tsbr[csbr].instr[cinst].type = MOV;
                    curr = currwrd;
                    cinst++;
                    return 1;
                }
                exit(1);
            } else if (check(t, Imm) || check(t, Div8)) {
                tsbr[csbr].instr[cinst].type = MOV;
                cinst++;
                return 1;
            } else if (validateAMEM(t, input)) {
                cinst++;
                return 1;
            }
            printf("Invalid 64-bit assignment\n");
            exit(1);
        }
    } else {
        if (OP(t)) {
            nextword(t, input);
            if (V32(t)) {
                tsbr[csbr].instr[cinst].op2 = currwrd;
                cinst++;
                return 1;
            }
            exit(1);
        } else if (check(t, Arrow)) {
            nextword(t, input);
            tsbr[csbr].instr[cinst].op1 = currwrd;
            if (check(t, R32)) {
                nextword(t, input);
                if (BO(t)) {
                    nextword(t, input);
                    if (V32(t)) {
                        tsbr[csbr].instr[cinst].op2 = currwrd;
                        cinst++;
                        return 1;
                    }
                    exit(1);
                } else if (check(t, Newline)) {
                    tsbr[csbr].instr[cinst].type = MOV;
                    curr = currwrd;
                    cinst++;
                    return 1;
                }
                exit(1);
            } else if (check(t, Imm) || check(t, Div8)) {
                tsbr[csbr].instr[cinst].type = MOV;
                cinst++;
                return 1;
            }
            exit(1);
        } else if (validateAMEM(t, input)) {
            cinst++;
            return 1;
        }
        exit(1);
    }
    return 0;
}

int validateAMEM(TA1STATE *t, char *input) {
    if (!check(t, MEM)) return 0;
    tsbr[csbr].instr[cinst].type = LDR;
    nextword(t, input);
    tsbr[csbr].instr[cinst].op1 = currwrd;
    if (check(t, FP)) {
        nextword(t, input);
        if (!check(t, Div8)) exit(1);
        tsbr[csbr].instr[cinst].op2 = currwrd;
        cinst++;
        return 1;
    } else if (currReg == 1 && check(t, R64)) {
        nextword(t, input);
        if (!check(t, Div8)) exit(1);
        tsbr[csbr].instr[cinst].op2 = currwrd;
        cinst++;
        return 1;
    } else if (currReg == 0 && check(t, R32)) {
        nextword(t, input);
        if (!check(t, Div8)) exit(1);
        tsbr[csbr].instr[cinst].op2 = currwrd;
        cinst++;
        return 1;
    }
    printf("Invalid MEM (load) instruction\n");
    exit(1);
}

int validateMEM(TA1STATE *t, char *input) {
    if (!check(t, MEM)) return 0;
    tsbr[csbr].instr[cinst].type = STR;
    nextword(t, input);
    tsbr[csbr].instr[cinst].dest = currwrd;
    if (check(t, FP)) {
        nextword(t, input);
        if (!check(t, Div8)) exit(1);
        tsbr[csbr].instr[cinst].op2 = currwrd;
        nextword(t, input);
        if (!check(t, Arrow)) exit(1);
        nextword(t, input);
        if (!check(t, R64) && !check(t, R32)) exit(1);
        tsbr[csbr].instr[cinst].op1 = currwrd;
        cinst++;
        return 1;
    } else if (check(t, R64) || check(t, R32)) {
        currReg = check(t, R64) ? 1 : 0;
        nextword(t, input);
        if (!check(t, Div8)) exit(1);
        tsbr[csbr].instr[cinst].op2 = currwrd;
        nextword(t, input);
        if (!check(t, Arrow)) exit(1);
        nextword(t, input);
        tsbr[csbr].instr[cinst].op1 = currwrd;
        if (currReg == 1) {
            if (!V64(t)) exit(1);
        } else {
            if (!check(t, R32)) exit(1);
        }
        cinst++;
        return 1;
    }
    printf("Invalid MEM (store) instruction\n");
    exit(1);
}

int validateLabel(TA1STATE *t, char *input) {
    if (!check(t, LabelTok)) return 0;
    tsbr[csbr].instr[cinst].type = LABEL;
    tsbr[csbr].instr[cinst].lbl  = currwrd;
    int idx = isExist(labels, t);
    if (idx >= 0)
        labels[idx].isSet = 1;
    else {
        store(labels, t, labelsCount, 1);
        labelsCount++;
    }
    nextword(t, input);
    if (check(t, Newline)) {
        printf("Invalid Label: no instruction follows on same line\n");
        exit(1);
    }
    cinst++;
    if (verifyLblInstr(t, input)) return 1;
    printf("Invalid label instruction\n");
    exit(1);
}

int validateBr(TA1STATE *t, char *input) {
    if (check(t, BR)) {
        tsbr[csbr].instr[cinst].type = B;
        nextword(t, input);
        if (!validateBrSbrlbl(t)) { printf("Invalid branch instruction\n"); exit(1); }
        cinst++;
        return 1;
    } else if (check(t, bl)) {
        tsbr[csbr].instr[cinst].type = BL;
        nextword(t, input);
        if (!validateBrSbrlbl(t)) { printf("Invalid bl instruction\n"); exit(1); }
        cinst++;
        return 1;
    }
    return 0;
}

int verifyLblInstr(TA1STATE *t, char *input) {
    return verifyCB(t, input) || A(t, input) || validateMEM(t, input)
        || validateRet(t) || validateBr(t, input);
}

int verifyInstructions(TA1STATE *t, char *input) {
    return verifyCB(t, input) || A(t, input) || validateMEM(t, input)
        || validateLabel(t, input) || validateRet(t) || validateBr(t, input);
}

void validateSbr(TA1STATE *t) {
    if (!check(t, Sbrname)) { printf("Expected subroutine name\n"); exit(1); }
    int idx = isExist(Sbr, t);
    if (idx >= 0 && Sbr[idx].isSet == 1) {
        printf("Duplicate subroutine declaration: %s\n", t->nw);
        exit(1);
    }
    store(Sbr, t, SbrCount, 1);
    SbrCount++;
}

int verifySbrDefn(TA1STATE *t, char *input) {
    if (!check(t, Oparen)) return 0;
    tsbr[csbr].totalInst = 0;
    labelsCount = 0;
    cinst = 0;
    nextword(t, input);
    validateSbr(t);
    tsbr[csbr].sbrIndex = currwrd;
    nextword(t, input); validate(t, Prmtr);
    nextword(t, input); validate(t, Newline);
    nextword(t, input);
    while (!check(t, Cparen)) {
        if (verifyInstructions(t, input)) {
            nextword(t, input);
            if (check(t, Newline)) {
                nextword(t, input);
                while (check(t, Newline)) nextword(t, input);
            } else {
                printf("Missing newline after instruction\n");
            }
        } else {
            printf("Invalid instruction inside subroutine\n");
            return 0;
        }
    }
    validate(t, Cparen);
    is_set(labels, labelsCount);
    tsbr[csbr].totalInst = cinst;
    csbr++;
    return 1;
}

void verify(TA1STATE *t, char *input) {
    nextword(t, input);
    if (!check(t, Oparen)) { printf("Expected '(' at program start\n"); exit(1); }
    nextword(t, input);
    validate(t, Sbrname);
    strcpy(s, t->nw);
    nextword(t, input); validate(t, Newline);
    nextword(t, input);
    while (!check(t, Cparen)) {
        if (verifySbrDefn(t, input)) {
            nextword(t, input);
            if (check(t, Newline)) {
                nextword(t, input);
                while (check(t, Newline)) nextword(t, input);
            }
        } else {
            printf("Invalid subroutine definition\n");
            exit(1);
        }
    }
    validate(t, Cparen);
    nextword(t, input);
    int found = 0;
    for (int i = 0; i < SbrCount; i++) {
        if (Sbr[i].name && strcmp(Sbr[i].name, s) == 0 && Sbr[i].isSet == 1) {
            printf("Entry subroutine '%s' is defined\n", s);
            found = 1;
            break;
        }
    }
    if (!found) { printf("Entry subroutine '%s' not found\n", s); exit(1); }
    if (check(t, EOT))
        printf("End of program\n");
    else
        printf("Warning: trailing tokens after program end\n");
}

static void tokenAt(int index, char *buf, int bufsize) {
    int i = 0;
    while (i < bufsize - 1 && input[index] != Null
           && input[index] != Space && input[index] != Tab) {
        buf[i++] = input[index++];
    }
    buf[i] = Null;
    if (buf[0] == '$' || buf[0] == ':') buf[0] = '_';
}

void translate(TSbr *tsbr) {
    tr = (char *)calloc(MAX_LEN, sizeof(char));
    if (!tr) { fprintf(stderr, "translate: malloc failed\n"); exit(1); }
    char tok[64];
    strcat(tr, ".section .text\n.global _start\n_start:\n");
    strcat(tr, "\tMOV X0, #1\n");
    strcat(tr, "\tBL __SBR");
    for (int i = 0; i < csbr; i++) {
        tokenAt(tsbr[i].sbrIndex, tok, sizeof(tok));
        char smod[50];
        strcpy(smod, s);
        smod[0] = '_';
        if (strcmp(tok, smod) == 0) {
            strcat(tr, "__SBR");
            strcat(tr, tok);
            break;
        }
    }
    strcat(tr, "\n\tMOV W8, #93\n\tSVC #0\n\tRET\n\n");
    for (int i = 0; i < csbr; i++) {
        tokenAt(tsbr[i].sbrIndex, tok, sizeof(tok));
        strcat(tr, "__SBR");
        strcat(tr, tok);
        strcat(tr, ":\n");
        for (int j = 0; j < tsbr[i].totalInst; j++) {
            char dst[64], o1[64], o2[64], lname[64], sname[64];
            switch (tsbr[i].instr[j].type) {
            case MOV:
                tokenAt(tsbr[i].instr[j].dest, dst, sizeof(dst));
                tokenAt(tsbr[i].instr[j].op1, o1, sizeof(o1));
                snprintf(tr + strlen(tr), MAX_LEN - strlen(tr), "\tMOV %s, %s\n", dst, o1);
                break;
            case ADD:
                tokenAt(tsbr[i].instr[j].dest, dst, sizeof(dst));
                tokenAt(tsbr[i].instr[j].op1, o1, sizeof(o1));
                tokenAt(tsbr[i].instr[j].op2, o2, sizeof(o2));
                snprintf(tr + strlen(tr), MAX_LEN - strlen(tr), "\tADD %s, %s, %s\n", dst, o1, o2);
                break;
            case SUB:
                tokenAt(tsbr[i].instr[j].dest, dst, sizeof(dst));
                tokenAt(tsbr[i].instr[j].op1, o1, sizeof(o1));
                tokenAt(tsbr[i].instr[j].op2, o2, sizeof(o2));
                snprintf(tr + strlen(tr), MAX_LEN - strlen(tr), "\tSUB %s, %s, %s\n", dst, o1, o2);
                break;
            case MUL:
                tokenAt(tsbr[i].instr[j].dest, dst, sizeof(dst));
                tokenAt(tsbr[i].instr[j].op1, o1, sizeof(o1));
                tokenAt(tsbr[i].instr[j].op2, o2, sizeof(o2));
                snprintf(tr + strlen(tr), MAX_LEN - strlen(tr), "\tMUL %s, %s, %s\n", dst, o1, o2);
                break;
            case LSL:
                tokenAt(tsbr[i].instr[j].dest, dst, sizeof(dst));
                tokenAt(tsbr[i].instr[j].op1, o1, sizeof(o1));
                tokenAt(tsbr[i].instr[j].op2, o2, sizeof(o2));
                snprintf(tr + strlen(tr), MAX_LEN - strlen(tr), "\tLSL %s, %s, %s\n", dst, o1, o2);
                break;
            case LSR:
                tokenAt(tsbr[i].instr[j].dest, dst, sizeof(dst));
                tokenAt(tsbr[i].instr[j].op1, o1, sizeof(o1));
                tokenAt(tsbr[i].instr[j].op2, o2, sizeof(o2));
                snprintf(tr + strlen(tr), MAX_LEN - strlen(tr), "\tLSR %s, %s, %s\n", dst, o1, o2);
                break;
            case LDR:
                tokenAt(tsbr[i].instr[j].dest, dst, sizeof(dst));
                tokenAt(tsbr[i].instr[j].op1, o1, sizeof(o1));
                tokenAt(tsbr[i].instr[j].op2, o2, sizeof(o2));
                snprintf(tr + strlen(tr), MAX_LEN - strlen(tr), "\tLDR %s, [%s, %s]\n", dst, o1, o2);
                break;
            case STR:
                tokenAt(tsbr[i].instr[j].op1, o1, sizeof(o1));
                tokenAt(tsbr[i].instr[j].dest, dst, sizeof(dst));
                tokenAt(tsbr[i].instr[j].op2, o2, sizeof(o2));
                snprintf(tr + strlen(tr), MAX_LEN - strlen(tr), "\tSTR %s, [%s, %s]\n", o1, dst, o2);
                break;
            case B:
                if (tsbr[i].instr[j].lbl > -1) {
                    tokenAt(tsbr[i].sbrIndex, tok, sizeof(tok));
                    tokenAt(tsbr[i].instr[j].lbl, lname, sizeof(lname));
                    snprintf(tr + strlen(tr), MAX_LEN - strlen(tr), "\tB __SBR%s%s\n", tok, lname);
                } else if (tsbr[i].instr[j].sbr > -1) {
                    tokenAt(tsbr[i].instr[j].sbr, sname, sizeof(sname));
                    snprintf(tr + strlen(tr), MAX_LEN - strlen(tr), "\tB __SBR%s\n", sname);
                }
                break;
            case BL:
                if (tsbr[i].instr[j].lbl > -1) {
                    tokenAt(tsbr[i].sbrIndex, tok, sizeof(tok));
                    tokenAt(tsbr[i].instr[j].lbl, lname, sizeof(lname));
                    snprintf(tr + strlen(tr), MAX_LEN - strlen(tr), "\tBL __SBR%s%s\n", tok, lname);
                } else if (tsbr[i].instr[j].sbr > -1) {
                    tokenAt(tsbr[i].instr[j].sbr, sname, sizeof(sname));
                    snprintf(tr + strlen(tr), MAX_LEN - strlen(tr), "\tBL __SBR%s\n", sname);
                }
                break;
            case BEQ: case BNE: case BGT: case BLT: case BGE: case BLE: {
                const char *mn =
                    (tsbr[i].instr[j].type == BEQ) ? "BEQ" :
                    (tsbr[i].instr[j].type == BNE) ? "BNE" :
                    (tsbr[i].instr[j].type == BGT) ? "BGT" :
                    (tsbr[i].instr[j].type == BLT) ? "BLT" :
                    (tsbr[i].instr[j].type == BGE) ? "BGE" : "BLE";
                tokenAt(tsbr[i].instr[j].op1, o1, sizeof(o1));
                tokenAt(tsbr[i].instr[j].op2, o2, sizeof(o2));
                snprintf(tr + strlen(tr), MAX_LEN - strlen(tr), "\tCMP %s, %s\n", o1, o2);
                if (tsbr[i].instr[j].sbr != -1) {
                    tokenAt(tsbr[i].instr[j].sbr, sname, sizeof(sname));
                    snprintf(tr + strlen(tr), MAX_LEN - strlen(tr), "\t%s __SBR%s\n", mn, sname);
                } else if (tsbr[i].instr[j].lbl != -1) {
                    tokenAt(tsbr[i].sbrIndex, tok, sizeof(tok));
                    tokenAt(tsbr[i].instr[j].lbl, lname, sizeof(lname));
                    snprintf(tr + strlen(tr), MAX_LEN - strlen(tr), "\t%s __SBR%s%s\n", mn, tok, lname);
                }
                break;
            }
            case LABEL:
                tokenAt(tsbr[i].sbrIndex, tok, sizeof(tok));
                tokenAt(tsbr[i].instr[j].lbl, lname, sizeof(lname));
                snprintf(tr + strlen(tr), MAX_LEN - strlen(tr), "__SBR%s%s:\n", tok, lname);
                break;
            case RETURN:
                strcat(tr, "\tBL __RET_ret\n");
                break;
            default:
                break;
            }
        }
        strcat(tr, "\tBL __RET_ret\n\n");
    }
    strcat(tr,
        "__RET_ret:\n"
        "\tMOV SP, FP\n"
        "\tLDR LR, [SP], #8\n"
        "\tLDR FP, [SP], #8\n"
        "\tRET\n"
    );
}
