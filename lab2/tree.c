#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include "tree.h"

#define print_tab(X)            \
    for (int i = 0; i < X; i++) \
    printf("  ")

node_t *root = NULL;

node_t *establish_leaf_node(node_type type, char *text)
{
    node_t *leaf = (node_t *)malloc(sizeof(node_t));
    for (int i = 0; i < MAX_CHILDNODE_NUM; i++)
        leaf->child_nodes[i] = NULL;
    assert(leaf != NULL);

    leaf->type = type;
    leaf->text = text;
    leaf->parent = NULL;
    leaf->child_nodes[0] = NULL;
    leaf->is_syntax = 0;
    // 词法单元节点未传入行号信息

    if (type == INT_T)
    {
        if (strlen(text) > 1 && text[1] == 'x')
            sscanf(text, "%x", &leaf->value.int_val);
        else if (text[0] == '0')
            sscanf(text, "%o", &leaf->value.int_val);
        else
            sscanf(text, "%d", &leaf->value.int_val);
    }
    else if (type == FLOAT_T)
    {
        sscanf(text, "%f", &leaf->value.float_val);
    }

    return leaf;
}

node_t *add_parent_node(char *text, int line, int child_count, ...)
{
    assert(child_count < MAX_CHILDNODE_NUM);

    node_t *node = (node_t *)malloc(sizeof(node_t));
    assert(node != NULL);
    for (int i = 0; i < MAX_CHILDNODE_NUM; i++)
        node->child_nodes[i] = NULL;

    va_list valist;
    va_start(valist, child_count);

    for (int i = 0, j = 0; i < child_count; i++, j++)
    {
        node_t *child = va_arg(valist, node_t *);
        if (child == NULL)
        {
            j--;
            continue;
        }

        node->child_nodes[j] = child;
        child->parent = node;
    }
    node->child_nodes[child_count] = NULL;
    node->type = COMMON_T;
    node->text = text;
    node->is_syntax = 1;
    node->line = line;

    va_end(valist);

    return node;
}

void print_tree(node_t *node, int depth)
{
    if (node == NULL)
        return;

    print_tab(depth);
    if (node->type == INT_T)
        printf("INT: %d", node->value.int_val);
    else if (node->type == ID_T)
        printf("ID: %s", node->text);
    else if (node->type == FLOAT_T)
        printf("FLOAT: %f", node->value.float_val);
    else if (node->type == TYPE_T)
        printf("TYPE: %s", node->text);
    else
        printf("%s", node->text);

    if (node->is_syntax)
        printf(" (%d)", node->line);

    printf("\n");

    for (int i = 0; node->child_nodes[i] != NULL; i++)
        print_tree(node->child_nodes[i], depth + 1);
}