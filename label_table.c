#include "label_table.h"

Label *Dlbl[TABLE_SIZE];
Label *CBlbl[TABLE_SIZE];
Label *Blbl[TABLE_SIZE];

void initLabels(void) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Dlbl[i]  = NULL;
        Blbl[i]  = NULL;
        CBlbl[i] = NULL;
    }
}

unsigned int hash(const char *str) {
    unsigned long h = 5381;
    int c;
    while ((c = (unsigned char)*str++))
        h = ((h << 5) + h) + c;
    return (unsigned int)(h % TABLE_SIZE);
}

void addLabel(const char *name, int instrNo, int labelType) {
    unsigned int index = hash(name);
    Label *node = (Label *)malloc(sizeof(Label));
    if (!node) { fprintf(stderr, "addLabel: malloc failed\n"); exit(1); }
    strncpy(node->name, name, NAME_LEN - 1);
    node->name[NAME_LEN - 1] = '\0';
    node->instrNo = instrNo;
    if (labelType == 0) {
        node->next  = Blbl[index];
        Blbl[index] = node;
    } else if (labelType == 1) {
        node->next  = Dlbl[index];
        Dlbl[index] = node;
    } else if (labelType == 2) {
        node->next   = CBlbl[index];
        CBlbl[index] = node;
    } else {
        fprintf(stderr, "addLabel: unknown labelType %d\n", labelType);
        free(node);
    }
}

Label *getLabel(const char *name, int labelType) {
    unsigned int index = hash(name);
    Label *temp = NULL;
    if      (labelType == 0) temp = Blbl[index];
    else if (labelType == 1) temp = Dlbl[index];
    else if (labelType == 2) temp = CBlbl[index];
    else return NULL;
    while (temp) {
        if (strcmp(temp->name, name) == 0) return temp;
        temp = temp->next;
    }
    return NULL;
}

void printTable(int labelType) {
    const char *names[] = { "Blbl", "Dlbl", "CBlbl" };
    printf("Table: %s\n", (labelType >= 0 && labelType <= 2) ? names[labelType] : "?");
    for (int i = 0; i < TABLE_SIZE; i++) {
        Label *temp = NULL;
        if      (labelType == 0) temp = Blbl[i];
        else if (labelType == 1) temp = Dlbl[i];
        else if (labelType == 2) temp = CBlbl[i];
        while (temp) {
            printf("  [%2d] %-16s %d\n", i, temp->name, temp->instrNo);
            temp = temp->next;
        }
    }
    printf("\n");
}

Var *VarTbl[VAR_TABLE_SIZE];
int  regPool[POOL_SIZE]  = { 9, 10, 11, 12, 13, 14, 15, 19 };
int  regFree[POOL_SIZE]  = { 1,  1,  1,  1,  1,  1,  1,  1 };
char *regOwner[POOL_SIZE]= { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
int  nextSpillOffset     = 8;

void initVars(void) {
    for (int i = 0; i < VAR_TABLE_SIZE; i++)
        VarTbl[i] = NULL;
    nextSpillOffset = 8;
}

static unsigned int varHash(const char *name) {
    unsigned long h = 5381;
    int c;
    while ((c = (unsigned char)*name++))
        h = ((h << 5) + h) + c;
    return (unsigned int)(h % VAR_TABLE_SIZE);
}

void addVar(const char *name, VarType type) {
    if (getVar(name)) {
        fprintf(stderr, "addVar: '%s' already declared\n", name);
        return;
    }
    unsigned int idx = varHash(name);
    Var *node = (Var *)malloc(sizeof(Var));
    if (!node) { fprintf(stderr, "addVar: malloc failed\n"); exit(1); }
    strncpy(node->name, name, NAME_LEN - 1);
    node->name[NAME_LEN - 1] = '\0';
    node->type      = type;
    node->reg       = allocReg(name);
    node->fp_offset = (node->reg == REG_NONE) ? nextSpillOffset : 0;
    if (node->reg == REG_NONE) nextSpillOffset += 8;
    node->next  = VarTbl[idx];
    VarTbl[idx] = node;
}

Var *getVar(const char *name) {
    unsigned int idx = varHash(name);
    Var *cur = VarTbl[idx];
    while (cur) {
        if (strcmp(cur->name, name) == 0) return cur;
        cur = cur->next;
    }
    return NULL;
}

void printVars(void) {
    printf("Variable Table:\n");
    for (int i = 0; i < VAR_TABLE_SIZE; i++) {
        Var *cur = VarTbl[i];
        while (cur) {
            const char *ts = (cur->type == TYPE_INT64) ? "int64" :
                             (cur->type == TYPE_INT32) ? "int32" : "ptr";
            if (cur->reg != REG_NONE)
                printf("  %-16s  %-6s  X%d\n", cur->name, ts, cur->reg);
            else
                printf("  %-16s  %-6s  spill  #%d\n", cur->name, ts, cur->fp_offset);
            cur = cur->next;
        }
    }
    printf("\n");
}

int allocReg(const char *varName) {
    for (int i = 0; i < POOL_SIZE; i++) {
        if (regFree[i]) {
            regFree[i]  = 0;
            regOwner[i] = strdup(varName);
            if (!regOwner[i]) { fprintf(stderr, "allocReg: strdup failed\n"); exit(1); }
            return regPool[i];
        }
    }
    fprintf(stderr, "allocReg: pool exhausted for '%s'\n", varName);
    return REG_NONE;
}

void freeReg(int reg) {
    for (int i = 0; i < POOL_SIZE; i++) {
        if (regPool[i] == reg && !regFree[i]) {
            regFree[i] = 1;
            free(regOwner[i]);
            regOwner[i] = NULL;
            return;
        }
    }
    fprintf(stderr, "freeReg: X%d not in pool\n", reg);
}

void freeRegByName(const char *varName) {
    for (int i = 0; i < POOL_SIZE; i++) {
        if (!regFree[i] && regOwner[i] && strcmp(regOwner[i], varName) == 0) {
            regFree[i] = 1;
            free(regOwner[i]);
            regOwner[i] = NULL;
            return;
        }
    }
    fprintf(stderr, "freeRegByName: '%s' not found\n", varName);
}

void printRegState(void) {
    printf("Register Pool:\n");
    for (int i = 0; i < POOL_SIZE; i++) {
        if (regFree[i])
            printf("  X%-2d  free\n", regPool[i]);
        else
            printf("  X%-2d  '%s'\n", regPool[i], regOwner[i] ? regOwner[i] : "?");
    }
    printf("\n");
}
