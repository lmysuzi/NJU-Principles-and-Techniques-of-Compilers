#include "data_structure.h"

ListNode *listnode_create(void *data)
{
    ListNode *node = (ListNode *)malloc(sizeof(ListNode));
    assert(node != NULL);
    node->data = data;
    return node;
}

ListNode *list_create()
{
    ListNode *head = listnode_create(NULL);
    head->next = head;
    head->prev = head;
    return head;
}

// append by ListNode
ListNode *list_append(ListNode *head, ListNode *node)
{
    assert(head != NULL && node != NULL);
    node->next = head;
    node->prev = head->prev;
    head->prev->next = node;
    head->prev = node;
    return head;
}

ListNode *list_append_by_data(ListNode *head, void *data)
{
    assert(head != NULL);
    ListNode *node = listnode_create(data);
    head = list_append(head, node);
    return head;
}

ListNode *list_search_by_key(ListNode *head, int (*cmp)(ListNode *, void *), void *key)
{
    assert(head != NULL);
    ListNode *cur = head->next;
    while (cur != head)
    {
        if (cmp(cur, key))
            return cur;
        cur = cur->next;
    }
    return NULL;
}

ListNode *list_search_by_data(ListNode *head, void *data, int (*cmp)(void *, void *))
{
    assert(head != NULL);
    ListNode *cur = head->next;
    while (cur != head)
    {
        if (cmp(data, cur->data))
            return cur;
        cur = cur->next;
    }
    return NULL;
}

ListNode *list_sub_data(ListNode *head, void *data, int (*cmp)(void *, void *))
{
    assert(head != NULL);
    ListNode *cur = head->next;
    while (cur != head)
    {
        ListNode *next = cur->next;
        if (cmp(data, cur->data))
        {
            cur->prev->next = cur->next;
            cur->next->prev = cur->prev;
            free(cur);
        }
        cur = next;
    }
    return head;
}

ListNode *list_teardown(ListNode *head)
{
    assert(head != NULL);
    ListNode *cur = head->next;
    while (cur != head)
    {
        ListNode *next = cur->next;
        free(cur);
        cur = next;
    }
    free(head);
    return NULL;
}

Set *set_create()
{
    Set *set = (Set *)malloc(sizeof(Set));
    assert(set != NULL);
    set->head = list_create();
    return set;
}

Set *set_copy(Set *a)
{
    Set *set = set_create();
    assert(a != NULL && set != NULL);
    ListNode *cur = a->head->next;
    while (cur != a->head)
    {
        set->head = list_append_by_data(set->head, cur->data);
        cur = cur->next;
    }
    return set;
}

Set *set_union(Set *a, Set *b, int (*cmp)(void *, void *))
{
    Set *set = set_copy(a);
    ListNode *cur = b->head->next;
    while (cur != b->head)
    {
        ListNode *search = list_search_by_data(a->head, cur->data, cmp);
        if (search == NULL)
        {
            set->head = list_append_by_data(set->head, cur->data);
        }
        cur = cur->next;
    }
    return set;
}

Set *set_intersect(Set *a, Set *b, int (*cmp)(void *, void *))
{
    Set *set = set_create();
    ListNode *cur = a->head->next;
    while (cur != a->head)
    {
        ListNode *search = list_search_by_data(b->head, cur->data, cmp);
        if (search != NULL)
        {
            set->head = list_append_by_data(set->head, cur->data);
        }
        cur = cur->next;
    }
    return set;
}

Set *set_add(Set *set, void *data)
{
    ListNode *cur = set->head->next;
    while (cur != set->head)
    {
        if (cur->data == data)
            return set;
        cur = cur->next;
    }
    set->head = list_append_by_data(set->head, data);
    return set;
}

// 集合是否存在元素
void *set_contains(Set *set, void *data, int (*cmp)(void *, void *))
{
    ListNode *search = list_search_by_data(set->head, data, cmp);
    if (search == NULL)
        return NULL;
    else
        return search->data;
}

Set *set_sub_data(Set *set, void *data, int (*cmp)(void *, void *))
{
    set->head = list_sub_data(set->head, data, cmp);
    return set;
}

int set_equal(Set *a, Set *b)
{
    ListNode *cur = a->head->next;
    while (cur != a->head)
    {
        ListNode *search = b->head->next;
        int found = 0;
        while (search != b->head)
        {
            if (search->data == cur->data)
            {
                found = 1;
                break;
            }
            search = search->next;
        }
        if (!found)
            return 0;
        cur = cur->next;
    }
    cur = b->head->next;
    while (cur != b->head)
    {
        ListNode *search = a->head->next;
        int found = 0;
        while (search != a->head)
        {
            if (search->data == cur->data)
            {
                found = 1;
                break;
            }
            search = search->next;
        }
        if (!found)
            return 0;
        cur = cur->next;
    }
    return 1;
}

Set *set_teardown(Set *set)
{
    assert(set != NULL);
    set->head = list_teardown(set->head);
    free(set);
    return NULL;
}

// 创建队列
Queue *queue_create()
{
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    queue->head = list_create();
    return queue;
}

// 入列
void queue_push(Queue *queue, void *data)
{
    list_append_by_data(queue->head, data);
}

// 出列
void *queue_pop(Queue *queue)
{
    if (queue->head->next == queue->head)
        return NULL;

    ListNode *ret = queue->head->next;
    queue->head->next = queue->head->next->next;
    queue->head->next->prev = queue->head;
    void *data = ret->data;
    free(ret);
    return data;
}

// 是否包含某元素
int queue_contains(Queue *queue, void *data)
{
    ListNode *cur = queue->head->next;
    while (cur != queue->head)
    {
        if (cur->data == data)
            return 1;
        cur = cur->next;
    }
    return 0;
}

// 队列是否为空
int queue_empty(Queue *queue)
{
    if (queue->head != queue->head->next)
        return 0;

    return 1;
}

// 销毁队列
Queue *queue_teardown(Queue *queue)
{
    list_teardown(queue->head);
    free(queue);
    return NULL;
}

void string_split(char *src, const char *separator, char **dest, int *num)
{
    char *pNext;
    int count = 0;
    if (src == NULL || strlen(src) == 0)
        return;
    if (separator == NULL || strlen(separator) == 0)
        return;
    pNext = strtok(src, separator);
    while (pNext != NULL)
    {
        *dest++ = pNext;
        count++;
        pNext = strtok(NULL, separator);
    }
    *num = count;
}
