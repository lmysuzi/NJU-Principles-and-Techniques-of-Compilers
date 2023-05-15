#include "operand.h"

static ListNode *var_list;

void operand_initial()
{
    var_list = list_create();
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
        op = (Operand *)malloc(sizeof(Operand));
        op->type = IMMEDIATE_OP;
        op->name = NULL;
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
            op->dag_node = NULL;
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