#ifndef __HASH_H__
#define __HASH_H__

#include "table.h"
#include "list.h"
#define HASHSLOT_SIZE 0x3fff
#define MAX_NESTED_DEPTH 10

typedef struct hashtable_t
{
    listnode_t *hashslot[HASHSLOT_SIZE];
    listnode_t *stack[MAX_NESTED_DEPTH];
    int stack_frame;
} hashtable_t;

void hash_init(hashtable_t *hashtable);

tnode_t *hash_search(hashtable_t *hashtable, char *name, ttype type);

int hash_insert(hashtable_t *hashtable, tnode_t *tnode);

void hash_stack_increase(hashtable_t *hashtable);

void hash_stack_decrease(hashtable_t *hashtable);

#endif