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

ListNode *list_append(ListNode *head, ListNode *node)
{
    assert(head != NULL && node != NULL);
    node->next = head;
    node->prev = head->prev;
    head->prev->next = node;
    head->prev = node;
    return head;
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
