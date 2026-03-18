#ifndef NEEDS_H
#define NEEDS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "label_table.h"

#define plus    "+"
#define Null    '\0'
#define Space   ' '
#define Tab     '\t'
#define MAX_LEN 20000
#define minus   "-"
#define mul     "*"
#define marrow  "*<-"
#define sarrow  "-<-"
#define parrow  "+<-"
#define ls      "<<"
#define rs      ">>"
#define lsarrow "<<<-"
#define rsarrow ">><-"
#define open    "("
#define close   ")"
#define nwline  "\n"
#define br      "b"
#define arrow   "<-"
#define X       "X"
#define W       "W"
#define doller  "$"

typedef enum {
    INVALID, LABEL, MOV, ADD, MUL, SUB, STR, LDR,
    B, BL, BEQ, BNE, BGT, BLT, BGE, BLE, LSL, LSR, RETURN
} InstrType;

typedef enum {
    Unknown, Newline, LabelTok, Sbrname, Oparen, Cparen,
    Imm, Div8, R32, R64, MEM, Arrow, LS, RS, LArrow, RArrow,
    SP, Plus, Minus, Mul, bl, BR, RET, FP, PArrow, SArrow,
    MArrow, Prmtr, CB, EQ, NE, GT, LT, GE, LE, EOT
} wordType;

typedef struct {
    char     nw[50];
    wordType currentType;
} TA1STATE;

typedef struct {
    char *name;
    int   isSet;
} Names;

typedef struct {
    InstrType type;
    int lbl;
    int sbr;
    int op1;
    int op2;
    int dest;
} TInstr;

typedef struct {
    int    sbrIndex;
    TInstr instr[50];
    int    totalInst;
} TSbr;

extern int    curr;
extern int    wrd;
extern int    currwrd;
extern int    SbrCount;
extern int    labelsCount;
extern int    currReg;
extern int    totalSbr;
extern int    csbr;
extern int    cinst;
extern char  *tr;
extern char   input[];
extern char   s[50];
extern char   word[50];
extern TA1STATE t;
extern Names  labels[10];
extern Names  Sbr[10];
extern TSbr   tsbr[20];

wordType classifyWord(char *word);
void     nextword(TA1STATE *t, char *input);
void     printSbrNames(void);
void     printLabelNames(void);
int      check(TA1STATE *t, wordType w);
int      checkCO(TA1STATE *t);
int      V32(TA1STATE *t);
int      V64(TA1STATE *t);
int      OP(TA1STATE *t);
int      BO(TA1STATE *t);
void     validate(TA1STATE *t, wordType w);
int      isExist(Names *arr, TA1STATE *t);
void     store(Names *arr, TA1STATE *t, int count, int isset);
int      validateBrSbrlbl(TA1STATE *t);
int      verifyCB(TA1STATE *t, char *input);
int      A(TA1STATE *t, char *input);
int      validateAMEM(TA1STATE *t, char *input);
int      validateMEM(TA1STATE *t, char *input);
int      verifyLblInstr(TA1STATE *t, char *input);
int      verifySbrDefn(TA1STATE *t, char *input);
int      verifyInstructions(TA1STATE *t, char *input);
int      validateLabel(TA1STATE *t, char *input);
void     verify(TA1STATE *t, char *input);
int      validateRet(TA1STATE *t);
void     validateSbr(TA1STATE *t);
void     is_set(Names *nm, int count);
void     translate(TSbr *tsbr);
void     getWord(int index);

#endif
