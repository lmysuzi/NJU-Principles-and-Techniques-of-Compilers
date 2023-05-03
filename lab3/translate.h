#ifndef __TRANSLATE_H__
#define __TRANSLATE_H__
#include "table.h"

typedef struct Operand_
{
    enum
    {
        VARIABLE_OP,
        CONSTANT_OP,
        ADDRESS_OP,
        FUNC_OP,
        TEMP_OP,
        LABEL_OP /*tobecontinue*/
    } kind;
    // 变量          常量          地址        函数     临时变量  label
    union
    {
        int var_no; // 临时变量，label等的序号
        int value;  // 常量的值
        char *name; // 函数或变量的
        // tobecontinue
    };
    int size;
} Operand;

typedef struct Operands
{
    Operand *op;
    struct Operands *next, *prev;
} Operands;

typedef struct InterCode
{
    enum
    {
        ASSIGN_IC,
        ADD_IC,
        SUB_IC, //     赋值a=b,  赋值a=b+c，赋值a=b-c
        MUL_IC,
        DIV_IC,
        FUN_IC, // 赋值a=b*c,  赋值a=b/c，函数头
        RETURN_IC,
        READ_IC,
        WRITE_IC, // return语句， READ指令，WRITE指令
        LABEL_IC,
        IF_IC,
        GOTO_IC,
        ARG_IC,
        PARAM_IC,
        CALL_IC,
        DEC_IC,
        LEFT_STAR_IC,  //*x=y
        RIGHT_STAR_IC, // x=*y
        RIGHT_ADDR_IC, // x=&y+z
    } kind;
    // LABEL语句，IF——GOTO语句，GOTO语句
    // 赋值
    union
    {
        struct
        {
            Operand *left, *right;
        } assign; // 对应ASSIGN_IC
        struct
        {
            Operand *op1, *op2, *result;
        } binop; // 对应加减乘除
        struct
        {
            Operand *func;
        } func; // 后续都一一与上对应
        struct
        {
            Operand *place;
        } return_;
        struct
        {
            Operand *readed;
        } read_;
        struct
        {
            Operand *written;
        } write_;
        struct
        {
            Operand *label;
        } lab;
        struct
        {
            Operand *left, *right, *label;
            char *relop;
        } if_goto;
        struct
        {
            Operand *label;
        } goto_;
        struct
        {
            Operand *arg;
        } arg_;
        struct
        {
            Operand *field;
        } param_;
        struct
        {
            Operand *left;
            char *callee;
        } call_;
        struct
        {
            Operand *var;
            int size;
        } dec_;

        // tobecontinue
    };
} InterCode;

typedef struct InterCodes
{
    InterCode *code;
    struct InterCodes *prev, *next;
} InterCodes;

typedef struct Exp_info
{
    tnode_t *tnode;
    InterCodes *code;
    Operand *addr;
    char *name;
    int is_param;
} Exp_info;

void translate(node_t *root, char *file_name);
void print_translation(char *file_name);                                                     // 将所有中间代码输出到file_name文件中
void print_op(Operand *op);                                                                  // 打印出operand变量
InterCodes *append_code(InterCodes *code1, InterCodes *code2);                               // 返回两个代码按code1在前，code2在后拼接的结果
Operand *new_temp();                                                                         // 新建一个临时变量
InterCodes *new_intercodes(Operand *op1, Operand *op2, Operand *op3, int type, char *relop); // 新建一行代码，op1,op2,op3分别由其type
                                                                                             // 不同表示不同含义，relop用来传递if_goto语句的relop
Operand *new_const(int val);                                                                 // 新建一个值为val的常量
Operand *new_func(char *name);                                                               // 新建一个名字为name的函数
Operand *new_var(char *name);                                                                // 新建一个名字为name的变量
Operand *new_label();                                                                        // 新建一个label
Operands *new_operands(Operand *op);

void translate_Program(node_t *node);
InterCodes *translate_ExtDefList(node_t *node);
InterCodes *translate_ExtDef(node_t *node);
vtype translate_Specifier(node_t *node);
vtype translate_StructSpecifier(node_t *node);
InterCodes *translate_FunDec(node_t *node, vtype type);
InterCodes *translate_VarList(node_t *node, tnode_t *func_node);
InterCodes *translate_StmtList(node_t *node, vtype functype);
InterCodes *translate_Stmt(node_t *node, vtype functype);
InterCodes *translate_CompSt(node_t *node, vtype functype, int is_func);
InterCodes *translate_DefList(node_t *node);
InterCodes *translate_Def(node_t *node);
InterCodes *translate_DecList(node_t *node);
InterCodes *translate_Dec(node_t *node);
InterCodes *translate_VarDec(node_t *node, Operand *place, int is_assign);
Exp_info translate_Exp(node_t *node, Operand *place, int direction); // param：place表示用来回填的临时变量
                                                                     // direction用来表示解析次EXP所处的位置，其=0时表示是在stmt或Dec中调用的该EXP，其=-1时表示其是在赋值性质的EXP
                                                                     // 等号的左侧，其=1时表示其在赋值性质EXP等号的右侧，其为-1和1分别会导致生成的代码中place在等号的左侧或右侧
InterCodes *translate_Args(node_t *node, Operands *arg_list, FieldList_t *field);
InterCodes *translate_Cond(node_t *node, Operand *label_true, Operand *label_false);      // 对IF、 WHILE中的EXP的解析
InterCodes *other_case_for_Cond(node_t *node, Operand *label_true, Operand *label_false); // translate_cond中其他条件执行动作的抽象

#endif