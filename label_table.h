#ifndef LABEL_TABLE_H
#define LABEL_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE     32
#define NAME_LEN       32
#define VAR_TABLE_SIZE 32
#define REG_NONE       -1
#define POOL_SIZE       8

typedef struct Label {
    char name[NAME_LEN];
    int instrNo;
    struct Label *next;
} Label;

extern Label *Dlbl[TABLE_SIZE];
extern Label *CBlbl[TABLE_SIZE];
extern Label *Blbl[TABLE_SIZE];

typedef enum { TYPE_INT64, TYPE_INT32, TYPE_PTR } VarType;

typedef struct Var {
    char name[NAME_LEN];
    VarType type;
    int reg;
    int fp_offset;
    struct Var *next;
} Var;

extern Var  *VarTbl[VAR_TABLE_SIZE];
extern int   regPool[POOL_SIZE];
extern int   regFree[POOL_SIZE];
extern char *regOwner[POOL_SIZE];
extern int   nextSpillOffset;

unsigned int hash(const char *str);
void   initLabels(void);
void   addLabel(const char *name, int instrNo, int labelType);
Label *getLabel(const char *name, int labelType);
void   printTable(int labelType);

void  initVars(void);
void  addVar(const char *name, VarType type);
Var  *getVar(const char *name);
int   allocReg(const char *varName);
void  freeReg(int reg);
void  freeRegByName(const char *varName);
void  printVars(void);
void  printRegState(void);

#endif
