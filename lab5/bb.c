#include "bb.h"

ListNode *bb_list_head;

void BB_initial()
{
    bb_list_head = list_create();
}

BB *create_BB()
{
    BB *bb = malloc(sizeof(BB));
    bb->head = list_create();
    return bb;
}

void build_BBs(ListNode *ir_head)
{
    ListNode *cur_ir = ir_head->next;

    BB *last_bb = NULL;

    while (cur_ir != ir_head)
    {
        IR *ir = (IR *)cur_ir->data;
        assert(ir != NULL);

        ListNode *new_node = listnode_create(ir);

        if (ir->type == LABEL_IR || ir->type == FUNC_IR)
        {
            // new bb
            if (last_bb != NULL)
            {
                ListNode *listnode = listnode_create(last_bb);
                bb_list_head = list_append(bb_list_head, listnode);
                last_bb = NULL;
            }
            BB *new_bb = create_BB();
            new_bb->head = list_append(new_bb->head, new_node);
            last_bb = new_bb;

            cur_ir = cur_ir->next;
            continue;
        }

        if (last_bb == NULL)
        {
            last_bb = create_BB();
        }
        last_bb->head = list_append(last_bb->head, new_node);

        if (ir->type == GOTO_IR || ir->type == CONDITIONAL_GOTO_IR || ir->type == RETURN_IR)
        {
            // end bb
            ListNode *listnode = listnode_create(last_bb);
            bb_list_head = list_append(bb_list_head, listnode);
            last_bb = NULL;
        }
        cur_ir = cur_ir->next;
    }

    if (last_bb != NULL)
    {
        ListNode *listnode = listnode_create(last_bb);
        bb_list_head = list_append(bb_list_head, listnode);
        last_bb = NULL;
    }

    ListNode *cur = bb_list_head->next;

    while (cur != bb_list_head)
    {

        BB *b = (BB *)cur->data;
        ListNode *irs = b->head;
        ListNode *ir = irs->next;

        while (ir != irs)
        {
            IR *i = (IR *)ir->data;
            printf("%d ", i->type);
            ir = ir->next;
        }
        printf("\n");
        cur = cur->next;
    }
}