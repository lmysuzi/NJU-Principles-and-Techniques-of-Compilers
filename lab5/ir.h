#ifndef __IR_H__
#define __IR_H__

#include "operand.h"
#include "data_structure.h"

struct CFGnode;

typedef struct Exp
{
    Operand *left, *right;
    enum
    {
        ADD = 1,
        SUB,
        MUL,
        DIV
    } type;
} Exp;

typedef struct IR
{
    // 该IR是否是BB的leader
    int is_leader;

    enum
    {
        LABEL_IR = 1,
        FUNC_IR,
        ASSIGN_IR,
        BINARY_IR,
        GOTO_IR,
        CONDITIONAL_GOTO_IR,
        RETURN_IR,
        DEC_IR,
        ARG_IR,
        CALL_IR,
        PARAM_IR,
        READ_IR,
        WRITE_IR,
    } type;

    struct CFGnode *cfg_node;

    union
    {
        struct
        {
            char *label_name;

        } label_ir;
        struct
        {
            char *func_name;
        } func_ir;
        struct
        {
            Operand *left, *right;
        } assign_ir;
        struct
        {
            Exp *exp;
            Operand *left, *right1, *right2;
            Operand *exp_var;
        } binary_ir;
        struct
        {
            char *label_name;
        } goto_ir;
        struct
        {
            Operand *left, *right;
            char *label_name;
            char *relop;
        } conditional_goto_ir;
        struct
        {
            Operand *return_var;
        } return_ir;
        struct
        {
            Operand *var;
            int size;
        } dec_ir;
        struct
        {
            Operand *arg_var;
        } arg_ir;
        struct
        {
            Operand *left;
            char *func_name;
        } call_ir;
        struct
        {
            Operand *param_var;
        } param_ir;
        struct
        {
            Operand *read_var;
        } read_ir;
        struct
        {
            Operand *write_var;
        } write_ir;
    };
} IR;

void ir_extract(FILE *file);

void ir_init();

IR *ir_create(int type);

void fprintf_ir(FILE *file, IR *ir);

extern ListNode *label_list_head;

extern ListNode *func_list_head;

extern ListNode *exp_list_head;

#endif