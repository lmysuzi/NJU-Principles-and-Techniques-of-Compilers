#include "hash.h"
#include "table.h"
#include <stdio.h>

void hash_init(hashtable_t *hashtable)
{
    for (int i = 0; i < HASHSLOT_SIZE; i++)
        hashtable->hashslot[i] = NULL;
    for (int i = 0; i < MAX_NESTED_DEPTH; i++)
        hashtable->stack[i] = NULL;
    hashtable->stack_frame = 0;
}

unsigned int hash_pjw(char *name)
{
    unsigned int val = 0, i;
    for (; *name; ++name)
    {
        val = (val << 2) + *name;
        if (i = val & ~HASHSLOT_SIZE)
            val = (val ^ (i >> 12)) & HASHSLOT_SIZE;
    }
    return val;
}

void printslot(hashtable_t *hashtable, int key)
{
    listnode_t *cur = hashtable->hashslot[key];
    while (cur)
    {
        printf("%s %d\n", cur->tnode->name, cur->tnode->type);
        cur = cur->next;
    }
    printf("\n");
}

tnode_t *hash_search(hashtable_t *hashtable, char *name, ttype type)
{
    unsigned int hash_key = hash_pjw(name);
    return list_search(hashtable->hashslot[hash_key], name, type);
}

int hash_insert(hashtable_t *hashtable, tnode_t *tnode)
{
    tnode->nested_depth = hashtable->stack_frame;
    if (tnode->type != VAR && tnode->type != ARRAY && tnode->type != STRUCTUR_USE)
        tnode->nested_depth = -1;
    unsigned int key = hash_pjw(tnode->name);
    list_insert(&(hashtable->hashslot[key]), tnode);
    if (tnode->type == VAR || tnode->type == ARRAY || tnode->type == STRUCTUR_USE)
        list_insert(&(hashtable->stack[hashtable->stack_frame]), tnode);
    // printslot(hashtable, key);
    return key;
}

void hash_remove(hashtable_t *hashtable, tnode_t *tnode)
{
    int key = hash_pjw(tnode->name);
    list_remove_by_tnode(&(hashtable->hashslot[key]), tnode);
}

void hash_stack_increase(hashtable_t *hashtable)
{
   // hashtable->stack_frame++;
}

void hash_stack_decrease(hashtable_t *hashtable)
{
    /*
    listnode_t *cur = hashtable->stack[hashtable->stack_frame];
    while (cur)
    {
        hash_remove(hashtable, cur->tnode);
        listnode_t *next = cur->next;
        free(cur);
        cur = next;
    }
    hashtable->stack[hashtable->stack_frame] = NULL;
    hashtable->stack_frame--;
    */
}