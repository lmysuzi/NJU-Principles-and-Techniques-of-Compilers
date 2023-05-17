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
