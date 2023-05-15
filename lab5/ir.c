#include "ir.h"

ListNode *ir_extract(FILE *file, ListNode *head)
{
    char buf[1024];

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
        assert(ir != NULL);

        if (strcmp(params[0], "FUNCTION") == 0)
        {
            ir->type = FUNC_IR;
            ir->func_ir.func_name = (char *)malloc(strlen(params[1]) + 1);
            strcpy(ir->func_ir.func_name, params[1]);
        }
        else if (strcmp(params[0], "LABEL") == 0)
        {
            ir->type = LABEL_IR;
            ir->label_ir.label_name = (char *)malloc(strlen(params[1]) + 1);
            strcpy(ir->label_ir.label_name, params[1]);
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
    return head;
}