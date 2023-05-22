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

// 通过data添加节点
ListNode *list_append_by_data(ListNode *head, void *data);

// 销毁链表
ListNode *list_teardown(ListNode *head);

// 集合
typedef struct Set
{
    ListNode *head;
} Set;

// 创建集合
Set *set_create();

// 复制集合
Set *set_copy(Set *a);

// 集合相并
Set *set_union(Set *a, Set *b, int (*cmp)(void *, void *));

// 集合相交
Set *set_intersect(Set *a, Set *b, int (*cmp)(void *, void *));

// 集合添加元素
Set *set_add(Set *set, void *data);

// 集合删除元素
Set *set_sub_data(Set *set, void *data, int (*cmp)(void *, void *));

// 集合是否相等
int set_equal(Set *a, Set *b);

// 集合是否存在元素
// 存在，则返回相应的data
void *set_contains(Set *set, void *data, int (*cmp)(void *, void *));

// 销毁集合
Set *set_teardown(Set *set);

// 队列
typedef struct Queue
{
    ListNode *head;
} Queue;

// 创建队列
Queue *queue_create();

// 入列
void queue_push(Queue *queue, void *data);

// 出列
void *queue_pop(Queue *queue);

// 是否包含某元素
int queue_contains(Queue *queue, void *data);

// 队列是否为空
int queue_empty(Queue *queue);

// 销毁队列
Queue *queue_teardown(Queue *queue);

// 字符串
// 字符串裁剪
void string_split(char *src, const char *separator, char **dest, int *num);

#endif