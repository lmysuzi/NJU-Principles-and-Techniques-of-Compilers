#ifndef __TABLE_H__
#define __TABLE_H__
#include <stdlib.h>
#include "tree.h"
/*./parser test/test.cmm
./parser test/test1.cmm
./parser test/test2.cmm
./parser test/test3.cmm
./parser test/test4.cmm
./parser test/test5.cmm
./parser test/test6.cmm
./parser test/test7.cmm
./parser test/test8.cmm
./parser test/test9.cmm
./parser test/test10.cmm
./parser test/test11.cmm
./parser test/test12.cmm
./parser test/test13.cmm
./parser test/test14.cmm
./parser test/test15.cmm
./parser test/test16.cmm*/
typedef enum
{
    VAR = 1,
    FUNC,
    FUNC_DEC, // declared function
    STRUCTUR,
    STRUCTUR_USE, // 主要用于在函数间传递参数，不会对用来对结构体赋值
    ARRAY,
} ttype;

typedef enum
{
    VINT = 0,
    VFLOAT,
    VERROR, // 由两种变量计算而来或者没有定义
} vtype;

typedef struct func_t
{
    vtype returntype;
    struct FieldList_t *para;
} func_t;

typedef struct structure_valuet
{
    // int istype;//值为1表示是定义的结构体，为0表示是使用结构体实例化的变量
    char *struct_name; // 表示实例对应的结构体的名字
    struct FieldList_t *item;
} structure_t;

typedef struct tnode_t
{
    ttype type; // 函数或变量
    char *name; // 函数名或变量名
    int line;
    int nested_depth;
    union
    {
        struct
        {
            struct tnode_t *elem;
            int size;  // 即这一维度可以有多少个elem
            int demin; // 即维度
        } array;       // 这个定义可以参考讲义上的那副关于数组链表的图
        vtype vartype; // 变量种类
        func_t *func;
        structure_t *structure;
    };
} tnode_t;

typedef struct FieldList_t
{
    tnode_t *type;            // 域的类型,也就是表示这个参数是int,float还是struct
    struct FieldList_t *tail; // 下一个域
} FieldList_t;                // 就是用来保存函数的参数一类的

typedef struct table_t
{
    tnode_t *node;
    struct table_t *next;
    struct table_t *pre;
} table_t; // 符号表

table_t *init_table();
int insert_table(tnode_t *node);
tnode_t *search_table(tnode_t *search_item);

void handle_undef_func();

void semantic_analyse(node_t *root);
void Program(node_t *node);
void ExtDefList(node_t *node);
void ExtDef(node_t *node);
vtype Specifier(node_t *node);
vtype StructSpecifier(node_t *node);
void ExtDecList();
tnode_t *FunDec(node_t *node, vtype type);
void CompSt(node_t *node, vtype functype, int is_func);
FieldList_t *VarList(node_t *node);
tnode_t *ParamDec(node_t *node);
tnode_t *VarDec(node_t *node, vtype type, char *struct_name);
FieldList_t *DefList(node_t *node, ttype isstruc);
FieldList_t *Def(node_t *node, ttype isstruc);                                    // param:isstruc:表示等下分析到的变量的定义是结构体的域，还是单纯只是变量
FieldList_t *DecList(node_t *node, vtype type, ttype isstruc, char *struct_name); // param:type:表示等下分析到的变量的定义是整数还是浮点数
                                                                                  // struct_name:表示结构体实例化对应的结构体原名
FieldList_t *Dec(node_t *node, vtype type, ttype isstruc, char *struct_name);
void StmtList(node_t *node, vtype functype);
void Stmt(node_t *node, vtype functype);
tnode_t *Exp(node_t *node);
FieldList_t *Args(node_t *node);
#endif