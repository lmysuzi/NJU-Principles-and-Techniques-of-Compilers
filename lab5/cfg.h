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
    ListNode *in_fact;
    ListNode *out_fact;

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
} CFG;

void cfg_init();

void cfgs_build();

#endif