#ifndef __TREE_H__
#define __TREE_H__

#define MAX_CHILDNODE_NUM 10

typedef enum
{
    INT_T = 1,
    FLOAT_T,
    ID_T,
    TYPE_T,
    RELOP_T,
    COMMON_T,
} node_type;

typedef union value_t
{
    int int_val;
    float float_val;
    char *relop_text;
} value_t;

typedef struct node_t
{
    struct node_t *parent;
    struct node_t *child_nodes[MAX_CHILDNODE_NUM];
    char *text;
    node_type type;
    int line;      // 行号
    int is_syntax; // 1表示语法单元,0表示词法单元
    value_t value;
} node_t;

node_t *establish_leaf_node(node_type type, char *text);
node_t *add_parent_node(char *text, int line, int child_count, ...);
void print_tree(node_t *node, int depth);

extern node_t *root;

#endif