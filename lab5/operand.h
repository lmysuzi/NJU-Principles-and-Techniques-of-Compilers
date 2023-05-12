#ifndef __OPERAND_H__
#define __OPERAND_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Operand
{
    char *name;

    enum
    {
        // 常数
        IMMEDIATE_OP = 1,
        VAR_OP,
    } type;
} Operand;

Operand *operand_create(char *text);

#endif