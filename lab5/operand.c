#include "operand.h"

static ListNode *var_list;

// 立即数
static ListNode *imm_list;

void operand_initial()
{
    var_list = list_create();
    imm_list = list_create();
}

void sprintf_op(char *addr, Operand *op)
{
    assert(op != NULL);
    if (op->type == IMMEDIATE_OP)
        sprintf(addr, "#%d", op->val);
    else
        sprintf(addr, "%s", op->name);
}

static int operand_listnode_cmp(ListNode *listnode, void *key)
{
    assert(listnode != NULL);
    assert(key != NULL);

    char *name = (char *)key;

    Operand *op = (Operand *)listnode->data;
    assert(op != NULL);

    if (strcmp(op->name, name) == 0)
        return 1;
    return 0;
}

Operand *operand_create(char *text)
{
    Operand *op = NULL;

    if (text[0] == '#')
    {
        int val;
        sscanf(text + 1, "%d", &val);
        for (ListNode *cur = imm_list->next; cur != imm_list; cur = cur->next)
        {
            Operand *imm = (Operand *)cur->data;
            if (imm->val == val)
                return imm;
        }
        op = (Operand *)malloc(sizeof(Operand));
        op->type = IMMEDIATE_OP;
        op->name = NULL;
        op->val = val;
        op->attached_exp = NULL;
        ListNode *node = listnode_create(op);
        imm_list = list_append(imm_list, node);
    }
    else
    {
        char *name = (char *)malloc(strlen(text) + 1);
        int index = 0;
        if (text[0] == '&' || text[0] == '*')
            index++;
        strcpy(name, text + index);

        ListNode *node = list_search_by_key(var_list, operand_listnode_cmp, name);

        if (node == NULL)
        {
            op = (Operand *)malloc(sizeof(Operand));
            op->type = VAR_OP;
            op->name = name;
            op->attached_exp = list_create();
            // op->dag_node = NULL;
            ListNode *node = listnode_create(op);
            var_list = list_append(var_list, node);
        }
        else
        {
            op = (Operand *)node->data;
        }
    }
    return op;
}

Operand *operand_exp_create()
{
    static int id = 1;
    Operand *op = (Operand *)malloc(sizeof(Operand));
    op->attached_exp = NULL;
    op->name = malloc(10);
    sprintf(op->name, "exp_%d", id);
    id++;
    op->type = EXP;
    op->val = 0;
    return op;
}