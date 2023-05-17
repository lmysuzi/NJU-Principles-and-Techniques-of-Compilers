#ifndef __OPERAND_H__
#define __OPERAND_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_structure.h"
// #include "dag.h"

typedef struct Operand
{
    char *name;
    int val;
    // 当前变量关联的表达式
    ListNode *attached_exp;
    // 该操作数所对应的DAG节点
    // DAGnode *dag_node;

    enum
    {
        // 常数
        IMMEDIATE_OP = 1,
        VAR_OP,
    } type;
} Operand;

Operand *operand_create(char *text);

void operand_initial();

#endif