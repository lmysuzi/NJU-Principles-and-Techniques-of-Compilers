#ifndef __LIST_H__
#define __LIST_H__

#include "table.h"

typedef struct listnode_t
{
    tnode_t *tnode;
    struct listnode_t *prev;
    struct listnode_t *next;
} listnode_t;

void list_insert(listnode_t **head, tnode_t *node);

tnode_t *list_search(listnode_t *head, char *name, ttype type);

void list_remove(listnode_t **head, char *name);

void list_remove_by_tnode(listnode_t **head, tnode_t *tnode);

#endif