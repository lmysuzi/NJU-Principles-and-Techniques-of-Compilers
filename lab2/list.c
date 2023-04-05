#include "list.h"
#include <string.h>
#include <stdio.h>

void list_insert(listnode_t **head, tnode_t *node)
{
    listnode_t *inserted_node = (listnode_t *)malloc(sizeof(listnode_t));
    inserted_node->tnode = node;
    inserted_node->prev = NULL;
    inserted_node->next = *head;
    if (*head)
    {
        (*head)->prev = inserted_node;
    }
    *head = inserted_node;
}

void list_remove(listnode_t **head, char *name)
{
    listnode_t *cur = *head;
    while (cur)
    {
        listnode_t *next = cur->next;
        if (strcmp(cur->tnode->name, name) == 0)
        {
            if (cur->prev)
                cur->prev->next = cur->next;
            else
                *head = next;
            if (cur->next)
                cur->next->prev = cur->prev;
            free(cur);
        }
        cur = next;
    }
}

void list_remove_by_tnode(listnode_t **head, tnode_t *tnode)
{
    listnode_t *cur = *head;
    while (cur)
    {
        if (cur->tnode == tnode)
        {
            if (cur->prev)
                cur->prev->next = cur->next;
            else
                *head = cur->next;
            if (cur->next)
                cur->next->prev = cur->prev;
            free(cur);
            return;
        }
        cur = cur->next;
    }
}

tnode_t *list_search(listnode_t *head, char *name, ttype type)
{
    listnode_t *cur = head;
    while (cur)
    {
        if (strcmp(cur->tnode->name, name) == 0 && (cur->tnode->type == type || type == 0))
        {
            return cur->tnode;
        }
        cur = cur->next;
    }
    return NULL;
}