#include "ir.h"

// 所有表达式的集合
static ListNode *exp_list_head;
// 所有IR的集合
ListNode *ir_list_head;
// 所有label的集合
ListNode *label_list_head;
// 所有函数及其所有指令的集合
// data 为 Listnode
ListNode *func_list_head;

void ir_init()
{
    exp_list_head = list_create();
    ir_list_head = list_create();
    label_list_head = list_create();
    func_list_head = list_create();
}

// 判断当前表达式是否已存在
static Exp *is_exp_exist(int type, Operand *left, Operand *right)
{
    ListNode *cur = exp_list_head->next;
    for (; cur != exp_list_head; cur = cur->next)
    {
        Exp *exp = (Exp *)cur->data;

        if (exp->type != type)
            continue;
        if (exp->left == left && exp->right == right)
            return exp;
        if (type == ADD || type == MUL)
        {
            if (exp->left == right && exp->right == left)
                return exp;
        }
    }
    return NULL;
}

static Exp *exp_create(char *text, Operand *left, Operand *right)
{
    int type = 0;
    switch (text[0])
    {
    case '+':
        type = ADD;
        break;
    case '-':
        type = SUB;
        break;
    case '*':
        type = MUL;
        break;
    case '/':
        type = DIV;
        break;
    default:
        assert(0);
        break;
    }

    Exp *exp = is_exp_exist(type, left, right);
    if (exp == NULL)
    {
        exp = (Exp *)malloc(sizeof(Exp));
        assert(exp != NULL);
        exp->left = left;
        exp->right = right;
        exp->type = type;

        if (left->type == VAR_OP)
        {
            ListNode *left_node = listnode_create(exp);
            left->attached_exp = list_append(left->attached_exp, left_node);
        }
        if (right->type == VAR_OP)
        {
            ListNode *right_node = listnode_create(exp);
            right->attached_exp = list_append(right->attached_exp, right_node);
        }
        ListNode *node = listnode_create(exp);
        exp_list_head = list_append(exp_list_head, node);
    }

    return exp;
}

void ir_extract(FILE *file)
{
    char buf[1024];
    ListNode *head = NULL;

    while (fgets(buf, sizeof(buf), file) != NULL)
    {
        if (buf[strlen(buf) - 1] == '\n')
        {
            buf[strlen(buf) - 1] = '\0';
        }
        char *params[6];
        int param_count;
        string_split(buf, " ", params, &param_count);

        IR *ir = (IR *)malloc(sizeof(IR));
        ir->is_leader = 0;
        ir->cfg_node = NULL;
        assert(ir != NULL);

        if (strcmp(params[0], "FUNCTION") == 0)
        {
            head = list_create();
            func_list_head = list_append_by_data(func_list_head, head);
            ir->type = FUNC_IR;
            ir->func_ir.func_name = (char *)malloc(strlen(params[1]) + 1);
            strcpy(ir->func_ir.func_name, params[1]);
        }
        else if (strcmp(params[0], "LABEL") == 0)
        {
            ir->type = LABEL_IR;
            ir->label_ir.label_name = (char *)malloc(strlen(params[1]) + 1);
            strcpy(ir->label_ir.label_name, params[1]);
            ListNode *label_node = listnode_create(ir);
            label_list_head = list_append(label_list_head, label_node);
        }
        else if (strcmp(params[0], "GOTO") == 0)
        {
            ir->type = GOTO_IR;
            ir->goto_ir.label_name = (char *)malloc(strlen(params[1]) + 1);
            strcpy(ir->goto_ir.label_name, params[1]);
        }
        else if (strcmp(params[0], "IF") == 0)
        {
            ir->type = CONDITIONAL_GOTO_IR;
            ir->conditional_goto_ir.label_name = (char *)malloc(strlen(params[5]) + 1);
            strcpy(ir->conditional_goto_ir.label_name, params[5]);
            ir->conditional_goto_ir.left = operand_create(params[1]);
            ir->conditional_goto_ir.right = operand_create(params[3]);
        }
        else if (strcmp(params[0], "RETURN") == 0)
        {
            ir->type = RETURN_IR;
            ir->return_ir.return_var = operand_create(params[1]);
        }
        else if (strcmp(params[0], "DEC") == 0)
        {
            ir->type = DEC_IR;
        }
        else if (strcmp(params[0], "ARG") == 0)
        {
            ir->type = ARG_IR;
            ir->arg_ir.arg_var = operand_create(params[1]);
        }
        else if (strcmp(params[2], "CALL") == 0)
        {
            ir->type = CALL_IR;
            ir->call_ir.left = operand_create(params[0]);
            ir->call_ir.func_name = (char *)malloc(strlen(params[3]) + 1);
            strcpy(ir->call_ir.func_name, params[3]);
        }
        else if (strcmp(params[0], "PARAM") == 0)
        {
            ir->type = PARAM_IR;
            ir->param_ir.param_var = operand_create(params[1]);
        }
        else if (strcmp(params[0], "READ") == 0)
        {
            ir->type = READ_IR;
            ir->read_ir.read_var = operand_create(params[1]);
        }
        else if (strcmp(params[0], "WRITE") == 0)
        {
            ir->type = WRITE_IR;
            ir->write_ir.write_var = operand_create(params[1]);
        }
        else if (param_count == 5)
        {
            ir->type = BINARY_IR;
            ir->binary_ir.left = operand_create(params[0]);
            ir->binary_ir.right1 = operand_create(params[2]);
            ir->binary_ir.right2 = operand_create(params[4]);
            ir->binary_ir.exp = exp_create(params[3], ir->binary_ir.right1, ir->binary_ir.right2);
        }
        else if (param_count == 3)
        {
            ir->type = ASSIGN_IR;
            ir->assign_ir.left = operand_create(params[0]);
            ir->assign_ir.right = operand_create(params[2]);
        }

        ListNode *ir_node = listnode_create(ir);
        head = list_append(head, ir_node);
    }
}