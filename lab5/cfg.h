#ifndef __CFG_H__
#define __CFG_H__

#include "data_structure.h"
#include "ir.h"

typedef struct CFGnode
{
    IR *stmt;
    // data: CFGnode
    ListNode *successors;
    ListNode *predecessors;
    // data :fact
    Set *in_fact;
    Set *out_fact;

    // 判断该节点是不是死代码
    int dead;

    enum
    {
        NORMAL,
        ENTRY,
        EXIT,
    } type;
} CFGnode;

typedef struct CFG
{
    CFGnode *entry_node;
    CFGnode *exit_node;
    // entry 和 exit 不在list当中
    ListNode *cfgnode_list;
} CFG;

void cfg_init();

void cfgs_build();

void cfgs_output(FILE *file);

extern ListNode *cfgs_list_head;

#endif