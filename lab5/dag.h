#ifndef __DAG_H__
#define __DAG_H__

#include "data_structure.h"
#include "ir.h"

// BB内部的有向无环图

typedef struct DAGnode
{
    // 该节点所表示的变量
    ListNode *attached_vars;

    // 该节点可能的两个源节点
    struct DAGnode *source_node1, *source_node2;

    // 该节点可能的目标节点
    struct DAGnode *target_node;

} DAGnode;

typedef struct DAG
{
    // 根节点
    ListNode *root_nodes;

    // 所有节点
    ListNode *nodes;
} DAG;

#endif