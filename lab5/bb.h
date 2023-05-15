#ifndef __BB_H__
#define __BB_H__

// 描述basic blocks

#include "data_structure.h"
#include "ir.h"

typedef struct BB
{
    ListNode *head;
} BB;

// 生成新的BB数据结构
BB *create_BB();

// 建立BBs
void build_BBs(ListNode *ir_head);

void BB_initial();

#endif