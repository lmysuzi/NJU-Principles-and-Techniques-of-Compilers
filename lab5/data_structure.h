#ifndef __DATA_STRUCTURE_H__
#define __DATA_STRUCTURE_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 链表
typedef struct ListNode
{
    void *data;
    struct ListNode *prev, *next;
} ListNode;

ListNode *listnode_create(void *data);

ListNode *list_create();

// 添加结点
ListNode *list_append(ListNode *head, ListNode *node);

// 两链表拼接，释放原链表头
ListNode *list_merge(ListNode *node);

// 通过特定的值来查找结点
ListNode *list_search_by_key(ListNode *head, int (*cmp)(ListNode *, void *), void *key);

// 字符串
// 字符串裁剪
void string_split(char *src, const char *separator, char **dest, int *num);

#endif