#include "dag.h"

static DAG *dag_create()
{
    DAG *dag = (DAG *)malloc(sizeof(DAG));
    assert(dag != NULL);

    dag->nodes = list_create();
    dag->root_nodes = list_create();
    return dag;
}

static DAGnode *dag_node_create()
{
    DAGnode *dag_node = (DAGnode *)malloc(sizeof(DAGnode));
    assert(dag_node != NULL);

    dag_node->attached_vars = list_create();
    dag_node->source_node1 = NULL;
    dag_node->source_node2 = NULL;
    dag_node->target_node = NULL;
    return dag_node;
}

void dag_build_assign_ir(DAG *dag, IR *ir)
{
    Operand *left = ir->assign_ir.left;
    Operand *right = ir->assign_ir.right;

    if (right->dag_node == NULL)
    {
        right->dag_node = dag_node_create();
        ListNode *rightnode = listnode_create(right);
        right->dag_node->attached_vars = list_append(right->dag_node->attached_vars, rightnode);
    }

    left->dag_node = dag_node_create();
    ListNode *leftnode = listnode_create(left);
    left->dag_node->attached_vars = list_append(left->dag_node->attached_vars, leftnode);

    left->dag_node->source_node1 = right->dag_node;
    right->dag_node->target_node = left->dag_node;
}

void dag_build(ListNode *ir_list)
{
    DAG *dag = dag_create();
    assert(ir_list != NULL);
    ListNode *cur_node = ir_list->next;

    while (cur_node != ir_list)
    {
        IR *ir = (IR *)cur_node->data;
        assert(ir != NULL);

        switch (ir->type)
        {
        case ASSIGN_IR:
            dag_build_assign_ir(dag, ir);
            break;

        default:
            break;
        }
        cur_node = cur_node->next;
    }
}