#include "translate.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "hash.h"

static InterCodes *intercodes_head; // 中间代码的头
int temp_no = 0;                    // 临时变量的数量
int lab_no;                         // label的数量
FILE *file;                         // 保存中间代码的文件

void translate(node_t *root, char *file_name)
{
    translate_Program(root);
    print_translation(file_name);
}

void print_op(Operand *op)
{
    char name[32];
    if (op->kind == FUNC_OP)
    {
        sprintf(name, "%s", op->name);
    }
    else if (op->kind == TEMP_OP)
    {
        sprintf(name, "temp%d", op->var_no);
    }
    else if (op->kind == CONSTANT_OP)
    {
        sprintf(name, "#%d", op->value);
    }
    else if (op->kind == VARIABLE_OP)
    {
        sprintf(name, "var%s", op->name);
    }
    else if (op->kind == LABEL_OP)
    {
        sprintf(name, "label%d", op->var_no);
    }
    else
    {
        assert(0);
    }
    fputs(name, file);
}

void print_translation(char *file_name)
{
    if (!(file = fopen(file_name, "w")))
    {
        perror(file_name);
        return;
    }
    InterCodes *p = intercodes_head;
    while (p != NULL)
    {
        if (p->code->kind == FUN_IC)
        {
            fputs("FUNCTION ", file);
            print_op(p->code->func.func);
            fputs(" :\n", file);
        }
        else if (p->code->kind == RETURN_IC)
        {
            fputs("RETURN ", file);
            print_op(p->code->return_.place);
            fputs(" \n", file);
        }
        else if (p->code->kind == ASSIGN_IC)
        {
            print_op(p->code->assign.left);
            fputs(" := ", file);
            print_op(p->code->assign.right);
            fputs("\n", file);
        }
        else if (p->code->kind == ADD_IC ||
                 p->code->kind == SUB_IC ||
                 p->code->kind == MUL_IC ||
                 p->code->kind == DIV_IC)
        {
            print_op(p->code->binop.result);
            fputs(" := ", file);
            print_op(p->code->binop.op1);
            if (p->code->kind == ADD_IC)
            {
                fputs(" + ", file);
            }
            else if (p->code->kind == SUB_IC)
            {
                fputs(" - ", file);
            }
            else if (p->code->kind == MUL_IC)
            {
                fputs(" * ", file);
            }
            else if (p->code->kind == DIV_IC)
            {
                fputs(" / ", file);
            }
            print_op(p->code->binop.op2);
            fputs("\n", file);
        }
        else if (p->code->kind == READ_IC)
        {
            fputs("READ ", file);
            print_op(p->code->read_.readed);
            fputs("\n", file);
        }
        else if (p->code->kind == LABEL_IC)
        {
            fputs("LABEL ", file);
            print_op(p->code->lab.label);
            fputs(" :\n", file);
        }
        else if (p->code->kind == IF_IC)
        {
            fputs("IF ", file);
            print_op(p->code->if_goto.left);
            fputs(" ", file);
            fputs(p->code->if_goto.relop, file);
            fputs(" ", file);
            print_op(p->code->if_goto.right);
            fputs(" GOTO ", file);
            print_op(p->code->if_goto.label);
            fputs("\n", file);
        }
        else if (p->code->kind == GOTO_IC)
        {
            fputs("GOTO ", file);
            print_op(p->code->goto_.label);
            fputs("\n", file);
        }
        else if (p->code->kind == WRITE_IC)
        {
            fputs("WRITE ", file);
            print_op(p->code->write_.written);
            fputs("\n", file);
        }
        else if (p->code->kind == CALL_IC)
        {
            print_op(p->code->call_.left);
            fputs(" := CALL ", file);
            fputs(p->code->call_.callee, file);
            fputs("\n", file);
        }
        else if (p->code->kind == ARG_IC)
        {
            fputs("ARG ", file);
            print_op(p->code->arg_.arg);
            fputs("\n", file);
        }
        else if (p->code->kind == PARAM_IC)
        {
            fputs("PARAM ", file);
            print_op(p->code->param_.field);
            fputs("\n", file);
        }
        else if (p->code->kind == DEC_IC)
        {
            fputs("DEC ", file);
            print_op(p->code->dec_.var);
            fprintf(file, " %d\n", p->code->dec_.size);
        }
        else if (p->code->kind == RIGHT_ADDR_IC)
        {
            print_op(p->code->binop.result);
            fputs(" := &", file);
            print_op(p->code->binop.op1);
            if (p->code->binop.op2 != NULL)
            {
                fputs(" + ", file);
                print_op(p->code->binop.op2);
            }
            fputs("\n", file);
        }
        else if (p->code->kind == RIGHT_STAR_IC)
        {
            print_op(p->code->assign.left);
            fputs(" := *", file);
            print_op(p->code->assign.right);
            fputs("\n", file);
        }
        else if (p->code->kind == LEFT_STAR_IC)
        {
            fputs("*", file);
            print_op(p->code->assign.left);
            fputs(" := ", file);
            print_op(p->code->assign.right);
            fputs("\n", file);
        }
        else
        {
            assert(0);
        }
        p = p->next;
    }
    fclose(file);
}

void translate_Program(node_t *node)
{
    if (node != NULL)
    {
        intercodes_head = translate_ExtDefList(node->child_nodes[0]);
    }
}

InterCodes *translate_ExtDefList(node_t *node)
{
    InterCodes *return_code = NULL;
    if (node != NULL)
    {
        InterCodes *code1 = translate_ExtDef(node->child_nodes[0]);
        InterCodes *code2 = translate_ExtDefList(node->child_nodes[1]);
        code1 = append_code(code1, code2);
        return_code = code1;
    }
    return return_code;
}

InterCodes *translate_ExtDef(node_t *node)
{
    InterCodes *return_code = NULL;
    vtype functype = translate_Specifier(node->child_nodes[0]);
    if (strcmp(node->child_nodes[1]->text, "FunDec") == 0)
    {
        if (functype == VINT_VAR)
            functype = VINT;
        else if (functype == VFLOAT_VAR)
            functype = VFLOAT;
        InterCodes *code1 = translate_FunDec(node->child_nodes[1], functype);
        return_code = code1;

        if (strcmp(node->child_nodes[2]->text, "SEMI") == 0)
        {
            // impossibel to happen
        }
        else
        {
            InterCodes *code2 = translate_CompSt(node->child_nodes[2], functype, 1);
            code1 = append_code(code1, code2);
        }
    }
    else if (strcmp(node->child_nodes[1]->text, "SEMI") == 0)
    {
        // did not need to do anything here
    }
    else if (strcmp(node->child_nodes[1]->text, "ExtDecList") == 0)
    {
    }
    return return_code;
}

vtype translate_Specifier(node_t *node)
{
    if (node->child_nodes[0]->type == TYPE_T)
    {
        if (strcmp(node->child_nodes[0]->text, "int") == 0)
            return VINT_VAR;
        else if (strcmp(node->child_nodes[0]->text, "float") == 0)
            return VFLOAT_VAR;
    }
    else if (strcmp(node->child_nodes[0]->text, "StructSpecifier") == 0)
    {
        return translate_StructSpecifier(node->child_nodes[0]);
    }
}

vtype translate_StructSpecifier(node_t *node)
{
    /*
    char *struct_name = node->child_nodes[1]->child_nodes[0]->text;

    tnode_t *struct_node = hash_search(&symbol_hashtable, struct_name, 0);
    assert(struct_node != NULL);
    FieldList_t *fields = struct_node->structure->item;
    // compute offset
    */
    return VSTRUCT_VAR;
}

InterCodes *translate_FunDec(node_t *node, vtype type)
{
    tnode_t *func_node = hash_search(&symbol_hashtable, node->child_nodes[0]->text, 0);
    assert(func_node != NULL);
    Operand *func = new_func(func_node->name);
    InterCodes *code1 = new_intercodes(func, NULL, NULL, FUN_IC, NULL);
    if (strcmp(node->child_nodes[2]->text, "VarList") == 0)
    {
        InterCodes *code2 = translate_VarList(node->child_nodes[2], func_node);
        code1 = append_code(code1, code2);
    }
    return code1;
}

InterCodes *translate_VarList(node_t *node, tnode_t *func_node)
{
    InterCodes *code1 = NULL;
    FieldList_t *fields = func_node->func->para;
    FieldList_t *cur_field = fields;
    while (cur_field != NULL)
    {
        char *field_name = cur_field->type->name;
        Operand *var = new_var(field_name);
        InterCodes *code2 = new_intercodes(var, NULL, NULL, PARAM_IC, NULL);
        code1 = append_code(code1, code2);
        cur_field = cur_field->tail;
    }
    return code1;
}

InterCodes *translate_StmtList(node_t *node, vtype functype)
{
    if (node != NULL)
    {
        InterCodes *code1 = translate_Stmt(node->child_nodes[0], functype);
        InterCodes *code2 = translate_StmtList(node->child_nodes[1], functype);
        if (code1 == NULL)
        {
            return code2;
        }
        code1 = append_code(code1, code2);
        return code1;
    }
    return NULL;
}

InterCodes *translate_Stmt(node_t *node, vtype functype)
{
    if (strcmp(node->child_nodes[0]->text, "Exp") == 0)
    {
        InterCodes *code1 = translate_Exp(node->child_nodes[0], NULL, 0).code;
        return code1;
    }
    else if (strcmp(node->child_nodes[0]->text, "RETURN") == 0)
    {
        Operand *temp = new_temp();
        InterCodes *code1 = translate_Exp(node->child_nodes[1], temp, 1).code;
        InterCodes *code2 = new_intercodes(temp, NULL, NULL, RETURN_IC, NULL);
        code1 = append_code(code1, code2);
        return code1;
    }
    else if (strcmp(node->child_nodes[0]->text, "IF") == 0)
    {
        if (node->child_nodes[5] == NULL)
        {
            Operand *label1 = new_label();
            Operand *label2 = new_label();
            InterCodes *code1 = translate_Cond(node->child_nodes[2], label1, label2);
            InterCodes *code2 = new_intercodes(label1, NULL, NULL, LABEL_IC, NULL);
            InterCodes *code3 = translate_Stmt(node->child_nodes[4], functype);
            InterCodes *code4 = new_intercodes(label2, NULL, NULL, LABEL_IC, NULL);
            code3 = append_code(code3, code4);
            code2 = append_code(code2, code3);
            code1 = append_code(code1, code2);
            return code1;
        }
        else
        {
            Operand *label1 = new_label();
            Operand *label2 = new_label();
            Operand *label3 = new_label();
            InterCodes *code1 = translate_Cond(node->child_nodes[2], label1, label2);
            InterCodes *code2 = new_intercodes(label1, NULL, NULL, LABEL_IC, NULL);
            InterCodes *code3 = translate_Stmt(node->child_nodes[4], functype);
            InterCodes *code4 = new_intercodes(label3, NULL, NULL, GOTO_IC, NULL);
            InterCodes *code5 = new_intercodes(label2, NULL, NULL, LABEL_IC, NULL);
            InterCodes *code6 = translate_Stmt(node->child_nodes[6], functype);
            InterCodes *code7 = new_intercodes(label3, NULL, NULL, LABEL_IC, NULL);
            code6 = append_code(code6, code7);
            code5 = append_code(code5, code6);
            code4 = append_code(code4, code5);
            code3 = append_code(code3, code4);
            code2 = append_code(code2, code3);
            code1 = append_code(code1, code2);
            return code1;
        }
    }
    else if (strcmp(node->child_nodes[0]->text, "WHILE") == 0)
    {
        Operand *label1 = new_label();
        Operand *label2 = new_label();
        Operand *label3 = new_label();
        InterCodes *code1 = new_intercodes(label1, NULL, NULL, LABEL_IC, NULL);
        InterCodes *code2 = translate_Cond(node->child_nodes[2], label2, label3);
        InterCodes *code3 = new_intercodes(label2, NULL, NULL, LABEL_IC, NULL);
        InterCodes *code4 = translate_Stmt(node->child_nodes[4], functype);
        InterCodes *code5 = new_intercodes(label1, NULL, NULL, GOTO_IC, NULL);
        InterCodes *code6 = new_intercodes(label3, NULL, NULL, LABEL_IC, NULL);
        code5 = append_code(code5, code6);
        code4 = append_code(code4, code5);
        code3 = append_code(code3, code4);
        code2 = append_code(code2, code3);
        code1 = append_code(code1, code2);
        return code1;
    }
    else if (strcmp(node->child_nodes[0]->text, "CompSt") == 0)
    {
        InterCodes *code1 = translate_CompSt(node->child_nodes[0], functype, 0);
        return code1;
    }
    else
        assert(0);
}

InterCodes *translate_Cond(node_t *node, Operand *label_true, Operand *label_false)
{
    if (node->child_nodes[1] == NULL)
    {
        /*
        Exp → ID
            | INT
            | FLOAT   //impossible in here
        */
        return other_case_for_Cond(node, label_true, label_false);
    }
    else if (strcmp(node->child_nodes[0]->text, "Exp") == 0 && strcmp(node->child_nodes[2]->text, "Exp") == 0)
    {
        if (strcmp(node->child_nodes[1]->text, "LB") == 0)
        {
            /*
            Exp → Exp LB Exp RB
            */
            return other_case_for_Cond(node, label_true, label_false);
        }
        else
        {
            // 双目运算
            /*
            Exp → Exp ASSIGNOP Exp
                | Exp AND Exp
                | Exp OR Exp
                | Exp RELOP Exp
                | Exp PLUS Exp
                | Exp MINUS Exp
                | Exp STAR Exp
                | Exp DIV Exp
            */
            if (strcmp(node->child_nodes[1]->text, "RELOP") == 0)
            {
                Operand *t1 = new_temp();
                Operand *t2 = new_temp();
                InterCodes *code1 = translate_Exp(node->child_nodes[0], t1, 1).code;
                InterCodes *code2 = translate_Exp(node->child_nodes[2], t2, 1).code;
                InterCodes *code3 = new_intercodes(t1, t2, label_true, IF_IC, node->child_nodes[1]->value.relop_text);
                InterCodes *code4 = new_intercodes(label_false, NULL, NULL, GOTO_IC, NULL);
                append_code(code3, code4);
                append_code(code2, code3);
                append_code(code1, code2);
                return code1;
            }
            else if (strcmp(node->child_nodes[1]->text, "AND") == 0)
            {
                Operand *label1 = new_label();
                InterCodes *code1 = translate_Cond(node->child_nodes[0], label1, label_false);
                InterCodes *code2 = new_intercodes(label1, NULL, NULL, LABEL_IC, NULL);
                InterCodes *code3 = translate_Cond(node->child_nodes[2], label_true, label_false);
                code2 = append_code(code2, code3);
                code1 = append_code(code1, code2);
                return code1;
            }
            else if (strcmp(node->child_nodes[1]->text, "OR") == 0)
            {
                Operand *label1 = new_label();
                InterCodes *code1 = translate_Cond(node->child_nodes[0], label_true, label1);
                InterCodes *code2 = new_intercodes(label1, NULL, NULL, LABEL_IC, NULL);
                InterCodes *code3 = translate_Cond(node->child_nodes[2], label_true, label_false);
                code2 = append_code(code2, code3);
                code1 = append_code(code1, code2);
                return code1;
            }
            return other_case_for_Cond(node, label_true, label_false);
        }
    }
    else if (node->child_nodes[0]->type == ID_T)
    {
        // function
        /*
        Exp → ID LP Args RP
            | ID LP RP
        */
        return other_case_for_Cond(node, label_true, label_false);
    }
    else if (strcmp(node->child_nodes[0]->text, "LP") == 0)
    {
        /*
        Exp → LP Exp RP
        */
        return other_case_for_Cond(node, label_true, label_false);
    }
    else if (strcmp(node->child_nodes[1]->text, "Exp") == 0)
    {
        /*
        Exp → MINUS Exp
            | NOT Exp
        */
        if (strcmp(node->child_nodes[0]->text, "NOT") == 0)
        {
            return translate_Cond(node->child_nodes[1], label_false, label_true);
        }
        else
        {
            return other_case_for_Cond(node, label_true, label_false);
        }
    }
    else if (strcmp(node->child_nodes[1]->text, "DOT") == 0)
    {
        /*
        Exp → Exp DOT ID
        */
        return other_case_for_Cond(node, label_true, label_false);
    }
    else
        return other_case_for_Cond(node, label_true, label_false);
    return other_case_for_Cond(node, label_true, label_false);
}

InterCodes *other_case_for_Cond(node_t *node, Operand *label_true, Operand *label_false)
{
    Operand *t1 = new_temp();
    InterCodes *code1 = translate_Exp(node, t1, 1).code;
    Operand *const_0 = new_const(0);
    InterCodes *code2 = new_intercodes(t1, const_0, label_true, IF_IC, "!=");
    InterCodes *code3 = new_intercodes(label_false, NULL, NULL, GOTO_IC, NULL);
    code2 = append_code(code2, code3);
    code1 = append_code(code1, code2);
    return code1;
}

InterCodes *translate_CompSt(node_t *node, vtype functype, int is_func)
{
    InterCodes *code1 = NULL;
    for (int i = 1; (i < 3) && (node->child_nodes[i] != NULL); i++)
    {
        if (strcmp(node->child_nodes[i]->text, "DefList") == 0)
        {
            InterCodes *code2 = translate_DefList(node->child_nodes[i]); // 这里虽然输入VAR，但只是为了和STRUCTUR相区分
            code1 = append_code(code1, code2);
        }
        else if (strcmp(node->child_nodes[i]->text, "StmtList") == 0)
        {
            InterCodes *code2 = translate_StmtList(node->child_nodes[i], functype);
            code1 = append_code(code1, code2);
        }
    }
    return code1;
}

InterCodes *translate_DefList(node_t *node)
{
    InterCodes *code1 = NULL;
    if (node != NULL)
    {
        InterCodes *code2 = translate_Def(node->child_nodes[0]);
        InterCodes *code3 = translate_DefList(node->child_nodes[1]);
        code2 = append_code(code2, code3);
        code1 = append_code(code1, code2);
    }
    return code1;
}

InterCodes *translate_Def(node_t *node)
{
    vtype type = translate_Specifier(node->child_nodes[0]);
    return translate_DecList(node->child_nodes[1]); // 只有int才有可能被赋初值
}

InterCodes *translate_DecList(node_t *node)
{
    InterCodes *code1 = translate_Dec(node->child_nodes[0]);
    if (node->child_nodes[1] != NULL)
    {
        InterCodes *code2 = translate_DecList(node->child_nodes[2]);
        code1 = append_code(code1, code2);
    }
    return code1;
}

InterCodes *translate_Dec(node_t *node)
{
    if (node->child_nodes[1] != NULL)
    {
        Operand *temp = new_temp();
        InterCodes *code1 = translate_Exp(node->child_nodes[2], temp, 1).code;
        InterCodes *code2 = translate_VarDec(node->child_nodes[0], temp, 1);
        code1 = append_code(code1, code2);
        return code1;
    }
    else
    {
        InterCodes *code1 = translate_VarDec(node->child_nodes[0], NULL, 0);
    }
}

InterCodes *translate_VarDec(node_t *node, Operand *place, int is_assign)
{
    if (node->child_nodes[0]->type == ID_T)
    {
        tnode_t *id_node = hash_search(&symbol_hashtable, node->child_nodes[0]->text, 0);
        assert(id_node != NULL);

        if (id_node->type == ARRAY || id_node->type == STRUCTUR_USE)
        {
            if (is_assign == 1)
                assert(0);
            Operand *op = new_var(id_node->name);
            op->size = id_node->size;
            InterCodes *code1 = new_intercodes(op, NULL, NULL, DEC_IC, NULL);
            return code1;
        }
        else if (is_assign == 1)
        {
            Operand *op = new_var(node->child_nodes[0]->text);
            InterCodes *code1 = new_intercodes(op, place, NULL, ASSIGN_IC, NULL);
            return code1;
        }
        return NULL;
    }
    else
    {
        InterCodes *code1 = translate_VarDec(node->child_nodes[0], place, is_assign);
        return code1;
    }
}

static Exp_info translate_array_struct(node_t *node)
{
    Exp_info ret;

    ret.addr = NULL;
    ret.code = NULL;
    ret.name = NULL;
    ret.is_param = 0;
    ret.tnode = NULL;

    if (node->child_nodes[0]->type == ID_T)
    {
        // 数组和结构的id
        char *id_name = node->child_nodes[0]->text;
        tnode_t *id_node = hash_search(&symbol_hashtable, id_name, 0);
        assert(id_node != NULL);

        ret.is_param = id_node->is_param;
        ret.name = id_node->name;
        if (id_node->type == STRUCTUR_USE)
            ret.tnode = id_node;
        else if (id_node->type == ARRAY)
            ret.tnode = id_node->array.elem;
    }
    else if (strcmp(node->child_nodes[1]->text, "DOT") == 0)
    {
        // 处理 exp.id
        char *field_name = node->child_nodes[2]->text;
        Exp_info l1_info = translate_array_struct(node->child_nodes[0]);
        tnode_t *struct_node = l1_info.tnode;
        InterCodes *code1 = l1_info.code;
        Operand *l1_addr = l1_info.addr;

        assert(struct_node != NULL);

        assert(struct_node->type == STRUCTUR_USE);

        FieldList_t *field = struct_node->structure->item;
        assert(field != NULL);

        while (field != NULL)
        {
            if (strcmp(field->type->name, field_name) == 0)
                break;
            field = field->tail;
        }
        assert(field != NULL);

        Operand *index = new_const(field->offset);
        Operand *addr = new_temp();
        InterCodes *code2 = NULL;

        if (l1_addr != NULL)
        {
            code2 = new_intercodes(addr, l1_addr, index, ADD_IC, NULL);
        }
        else
        {
            code2 = new_intercodes(addr, index, NULL, ASSIGN_IC, NULL);
        }
        code1 = append_code(code1, code2);

        ret.addr = addr;
        ret.code = code1;
        ret.name = l1_info.name;
        if (field->type->type == ARRAY)
            ret.tnode = field->type->array.elem;
        else
            ret.tnode = field->type;
        ret.is_param = l1_info.is_param;
    }
    else if (strcmp(node->child_nodes[1]->text, "LB") == 0)
    {
        // 处理 exp[exp]
        Exp_info l1_info = translate_array_struct(node->child_nodes[0]);

        tnode_t *cur_node = l1_info.tnode;
        InterCodes *code1 = l1_info.code;
        Operand *l1_addr = l1_info.addr;

        Operand *addr = new_temp();
        Operand *width = new_const(cur_node->size);
        Operand *index = new_temp();

        InterCodes *code2 = translate_Exp(node->child_nodes[2], index, 1).code;
        InterCodes *code3 = NULL;

        if (l1_addr != NULL)
        {
            Operand *t = new_temp();
            code3 = new_intercodes(t, index, width, MUL_IC, NULL);
            InterCodes *code4 = new_intercodes(addr, l1_addr, t, ADD_IC, NULL);
            code3 = append_code(code3, code4);
        }
        else
        {
            code3 = new_intercodes(addr, index, width, MUL_IC, NULL);
        }
        code2 = append_code(code2, code3);
        code1 = append_code(code1, code2);

        ret.addr = addr;
        if (cur_node->type == ARRAY)
            ret.tnode = cur_node->array.elem;
        else
            ret.tnode = cur_node;
        ret.code = code1;
        ret.name = l1_info.name;
        ret.is_param = l1_info.is_param;
    }
    else
        assert(0);

    return ret;
}

Exp_info translate_Exp(node_t *node, Operand *place, int direction)
{
    Exp_info ret;
    ret.addr = NULL;
    ret.code = NULL;
    ret.is_param = 0;
    ret.name = NULL;
    ret.tnode = NULL;
    if (node->child_nodes[1] == NULL)
    {
        /*
        Exp → ID
            | INT
            | FLOAT   //impossible in here
        */
        if (node->child_nodes[0]->type == INT_T)
        {
            if (direction == 0)
                place = new_temp();
            if (direction != -1)
            {
                Operand *op = new_const(node->child_nodes[0]->value.int_val);
                InterCodes *code1 = new_intercodes(place, op, NULL, ASSIGN_IC, NULL);
                ret.code = code1;
                return ret;
            }
            else
            {
                assert(0);
            }
        }
        else if (node->child_nodes[0]->type == ID_T)
        {
            char *id_name = node->child_nodes[0]->text;
            tnode_t *id_node = hash_search(&symbol_hashtable, id_name, 0);
            assert(id_node != NULL);
            if (direction == 0)
                place = new_temp();
            if (id_node->type == ARRAY || id_node->type == STRUCTUR_USE)
            {
                assert(direction != -1);
                Operand *op = new_var(id_name);
                InterCodes *code1 = new_intercodes(place, op, NULL, RIGHT_ADDR_IC, NULL);
                ret.code = code1;
                return ret;
            }
            if (direction == -1)
            {
                Operand *op = new_var(node->child_nodes[0]->text);
                InterCodes *code1 = new_intercodes(op, place, NULL, ASSIGN_IC, NULL);
                ret.code = code1;
                return ret;
            }
            else
            {
                Operand *op = new_var(node->child_nodes[0]->text);
                InterCodes *code1 = new_intercodes(place, op, NULL, ASSIGN_IC, NULL);
                ret.code = code1;
                return ret;
            }
        }
        else
        {
            assert(0);
        }
    }
    else if (strcmp(node->child_nodes[0]->text, "Exp") == 0 && strcmp(node->child_nodes[2]->text, "Exp") == 0)
    {
        if (strcmp(node->child_nodes[1]->text, "LB") == 0)
        {
            /*
            Exp → Exp LB Exp RB
            */
            if (place == NULL)
                place = new_temp();

            Exp_info info = translate_array_struct(node);
            InterCodes *code1 = info.code;
            Operand *index = info.addr;
            Operand *array_base = new_var(info.name);
            Operand *addr = new_temp();
            InterCodes *code2 = NULL;
            if (info.is_param == IS_PARAM || direction == 2)
                code2 = new_intercodes(addr, array_base, index, ADD_IC, NULL);
            else
                code2 = new_intercodes(addr, array_base, index, RIGHT_ADDR_IC, NULL);
            InterCodes *code3 = NULL;
            if (direction == 1 || direction == 0)
            {
                code3 = new_intercodes(place, addr, NULL, RIGHT_STAR_IC, NULL);
            }
            else if (direction == -1)
            {
                code3 = new_intercodes(addr, place, NULL, LEFT_STAR_IC, NULL);
            }
            else if (direction == 2)
            {
                code3 = new_intercodes(place, addr, NULL, ASSIGN_IC, NULL);
            }

            code2 = append_code(code2, code3);
            code1 = append_code(code1, code2);
            ret.code = code1;
            return ret;
        }
        else
        {
            // 双目运算
            /*
            Exp → Exp ASSIGNOP Exp
                | Exp AND Exp
                | Exp OR Exp
                | Exp RELOP Exp
                | Exp PLUS Exp
                | Exp MINUS Exp
                | Exp STAR Exp
                | Exp DIV Exp
            */
            if (strcmp(node->child_nodes[1]->text, "ASSIGNOP") == 0)
            {
                if (direction == 0)
                {
                    Operand *temp = new_temp();
                    InterCodes *code1 = translate_Exp(node->child_nodes[2], temp, 1).code;
                    InterCodes *code2 = translate_Exp(node->child_nodes[0], temp, -1).code;
                    code1 = append_code(code1, code2);
                    ret.code = code1;
                    return ret;
                }
                else if (direction == 1 || direction == 2)
                {

                    Operand *temp = new_temp();
                    InterCodes *code1 = translate_Exp(node->child_nodes[2], temp, 1).code;
                    InterCodes *code2 = translate_Exp(node->child_nodes[0], temp, -1).code;
                    InterCodes *code3 = new_intercodes(place, temp, NULL, ASSIGN_IC, NULL);
                    code2 = append_code(code2, code3);
                    code1 = append_code(code1, code2);
                    ret.code = code1;
                    return ret;
                }
                else
                {
                    assert(0);
                }
            }
            else if (strcmp(node->child_nodes[1]->text, "PLUS") == 0 ||
                     strcmp(node->child_nodes[1]->text, "MINUS") == 0 ||
                     strcmp(node->child_nodes[1]->text, "STAR") == 0 ||
                     strcmp(node->child_nodes[1]->text, "DIV") == 0)
            {
                if (direction == 0)
                    place = new_temp();
                if (direction == 1 || direction == 0 || direction == 2)
                {
                    Operand *temp1 = new_temp();
                    Operand *temp2 = new_temp();
                    InterCodes *code1 = translate_Exp(node->child_nodes[0], temp1, 1).code;
                    InterCodes *code2 = translate_Exp(node->child_nodes[2], temp2, 1).code;
                    InterCodes *code3;
                    if (strcmp(node->child_nodes[1]->text, "PLUS") == 0)
                    {
                        code3 = new_intercodes(place, temp1, temp2, ADD_IC, NULL);
                    }
                    else if (strcmp(node->child_nodes[1]->text, "MINUS") == 0)
                    {
                        code3 = new_intercodes(place, temp1, temp2, SUB_IC, NULL);
                    }
                    else if (strcmp(node->child_nodes[1]->text, "STAR") == 0)
                    {
                        code3 = new_intercodes(place, temp1, temp2, MUL_IC, NULL);
                    }
                    else if (strcmp(node->child_nodes[1]->text, "DIV") == 0)
                    {
                        code3 = new_intercodes(place, temp1, temp2, DIV_IC, NULL);
                    }
                    code2 = append_code(code2, code3);
                    code1 = append_code(code1, code2);
                    ret.code = code1;
                    return ret;
                }
                else
                {
                    assert(0);
                }
            }
            else if (strcmp(node->child_nodes[1]->text, "RELOP") == 0 ||
                     strcmp(node->child_nodes[1]->text, "AND") == 0 ||
                     strcmp(node->child_nodes[1]->text, "OR") == 0)
            {
                if (place == NULL)
                    place = new_temp();
                Operand *label1 = new_label();
                Operand *label2 = new_label();
                InterCodes *code0 = new_intercodes(place, new_const(0), NULL, ASSIGN_IC, NULL);
                InterCodes *code1 = translate_Cond(node, label1, label2);
                InterCodes *code2 = new_intercodes(label1, NULL, NULL, LABEL_IC, NULL);
                InterCodes *code3 = new_intercodes(place, new_const(1), NULL, ASSIGN_IC, NULL);
                InterCodes *code4 = new_intercodes(label2, NULL, NULL, LABEL_IC, NULL);
                code3 = append_code(code3, code4);
                code2 = append_code(code2, code3);
                code1 = append_code(code1, code2);
                code0 = append_code(code0, code1);
                ret.code = code0;
                return ret;
            }
            else
                assert(0);
        }
    }
    else if (node->child_nodes[0]->type == ID_T)
    {
        // function
        /*
        Exp → ID LP Args RP
            | ID LP RP
        */
        if (direction == 0)
            place = new_temp();
        if (strcmp(node->child_nodes[0]->text, "read") == 0)
        {
            InterCodes *code1 = new_intercodes(place, NULL, NULL, READ_IC, NULL);
            ret.code = code1;
            return ret;
        }
        else if (strcmp(node->child_nodes[0]->text, "write") == 0)
        {
            Operands dummy;
            InterCodes *code1 = translate_Args(node->child_nodes[2], &dummy);
            InterCodes *code2 = new_intercodes(dummy.next->op, NULL, NULL, WRITE_IC, NULL);
            InterCodes *code3 = new_intercodes(place, new_const(0), NULL, ASSIGN_IC, NULL);
            code2 = append_code(code2, code3);
            code1 = append_code(code1, code2);
            ret.code = code1;
            return ret;
        }
        else if (strcmp(node->child_nodes[2]->text, "RP") == 0)
        {
            InterCodes *code1 = new_intercodes(place, NULL, NULL, CALL_IC, node->child_nodes[0]->text);
            ret.code = code1;
            return ret;
        }
        else if (strcmp(node->child_nodes[2]->text, "Args") == 0)
        {
            Operands dummy;
            dummy.next = dummy.prev = NULL;
            InterCodes *code1 = translate_Args(node->child_nodes[2], &dummy);
            InterCodes *code2 = NULL;
            Operands *arg = dummy.next;
            while (arg != NULL)
            {
                InterCodes *arg_code = new_intercodes(arg->op, NULL, NULL, ARG_IC, NULL);
                code2 = append_code(code2, arg_code);
                arg = arg->next;
            }
            InterCodes *code3 = new_intercodes(place, NULL, NULL, CALL_IC, node->child_nodes[0]->text);
            code2 = append_code(code2, code3);
            code1 = append_code(code1, code2);
            ret.code = code1;
            return ret;
        }
        else
            assert(0);
    }
    else if (strcmp(node->child_nodes[0]->text, "LP") == 0)
    {
        /*
        Exp → LP Exp RP
        */
        if (direction == 1 || direction == 2)
        {
            Operand *temp1 = new_temp();
            InterCodes *code1 = translate_Exp(node->child_nodes[1], temp1, 1).code;
            InterCodes *code2 = new_intercodes(place, temp1, NULL, ASSIGN_IC, NULL);
            code1 = append_code(code1, code2);
            ret.code = code1;
            return ret;
        }
        else if (direction == 0)
        {
            Operand *temp1 = new_temp();
            InterCodes *code1 = translate_Exp(node->child_nodes[1], temp1, 1).code;
            ret.code = code1;
            return ret;
        }
        else
        {
            assert(0);
        }
    }
    else if (strcmp(node->child_nodes[1]->text, "Exp") == 0)
    {
        /*
        Exp → MINUS Exp
            | NOT Exp
        */
        if (strcmp(node->child_nodes[0]->text, "MINUS") == 0)
        {
            if (direction == 1 || direction == 2)
            {
                Operand *temp1 = new_temp();
                InterCodes *code1 = translate_Exp(node->child_nodes[1], temp1, 1).code;
                Operand *const_0 = new_const(0);
                InterCodes *code2 = new_intercodes(place, const_0, temp1, SUB_IC, NULL);
                code1 = append_code(code1, code2);
                ret.code = code1;
                return ret;
            }
            else if (direction == 0)
            {
                Operand *temp1 = new_temp();
                InterCodes *code1 = translate_Exp(node->child_nodes[1], temp1, 0).code;
                ret.code = code1;
                return ret;
            }
            else
            {
                assert(0);
            }
        }
        else
        {
            assert(0);
        }
    }
    else if (strcmp(node->child_nodes[1]->text, "DOT") == 0)
    {
        /*
        Exp → Exp DOT ID
        */
        Exp_info struct_info = translate_array_struct(node);
        char *var_name = struct_info.name;
        InterCodes *code1 = struct_info.code;
        Operand *index = struct_info.addr;
        Operand *struct_base = new_var(var_name);
        Operand *addr = new_temp();
        InterCodes *code2 = NULL;
        if (struct_info.is_param == IS_PARAM || direction == 2)
            code2 = new_intercodes(addr, struct_base, index, ADD_IC, NULL);
        else
            code2 = new_intercodes(addr, struct_base, index, RIGHT_ADDR_IC, NULL);
        InterCodes *code3 = NULL;
        if (direction == 1 || direction == 0)
        {
            code3 = new_intercodes(place, addr, NULL, RIGHT_STAR_IC, NULL);
        }
        else if (direction == -1)
        {
            code3 = new_intercodes(addr, place, NULL, LEFT_STAR_IC, NULL);
        }
        else if (direction == 2)
        {
            code3 = new_intercodes(place, addr, NULL, ASSIGN_IC, NULL);
        }
        code2 = append_code(code2, code3);
        code1 = append_code(code1, code2);
        ret.code = code1;
        return ret;
    }
    else
        assert(0);
}

InterCodes *translate_Args(node_t *node, Operands *arg_list)
{
    /*
    FieldList_t *para = (FieldList_t *)malloc(sizeof(FieldList_t));
    exp_t ex = Exp(node->child_nodes[0]);
    tnode_t *para_node = ex.symbol;

    if (para_node == NULL)
    {
        // 右值
        // 此处假设右值全为int float
        para_node = (tnode_t *)malloc(sizeof(tnode_t));
        para_node->vartype = ex.type;
        if (ex.type == VSTRUCT_VAR || ex.type == VARRAY)
            assert(0);
        para_node->type = VAR;
    }
    para->type = para_node;
    if (node->child_nodes[1] != NULL)
    {
        para->tail = Args(node->child_nodes[2]);
    }
    return para;
    */
    Operand *t1 = new_temp();
    Operands *arg = new_operands(t1);
    arg->next = arg_list->next;
    arg->prev = arg_list;
    arg_list->next = arg;
    InterCodes *code1 = translate_Exp(node->child_nodes[0], t1, 2).code;
    if (node->child_nodes[1] != NULL)
    {
        InterCodes *code2 = translate_Args(node->child_nodes[2], arg_list);
        append_code(code1, code2);
    }
    return code1;
}

InterCodes *append_code(InterCodes *code1, InterCodes *code2)
{
    if (code1 == NULL)
    {
        return code2;
    }
    if (code2 == NULL)
    {
        return code1;
    }
    InterCodes *p = code1;
    while (p->next != NULL)
    {
        p = p->next;
    }
    p->next = code2;
    code2->prev = p;
    return code1;
}

Operand *new_temp()
{
    Operand *temp = (Operand *)malloc(sizeof(Operand));
    temp->kind = TEMP_OP;
    temp->var_no = temp_no;
    temp_no++;
    return temp;
}

Operand *new_const(int val)
{
    Operand *op = (Operand *)malloc(sizeof(Operand));
    op->kind = CONSTANT_OP;
    op->value = val;
    return op;
}

Operand *new_func(char *name)
{
    Operand *func = (Operand *)malloc(sizeof(Operand));
    func->kind = FUNC_OP;
    func->name = name;
    return func;
}

Operand *new_var(char *name)
{
    Operand *var = (Operand *)malloc(sizeof(Operand));
    var->kind = VARIABLE_OP;
    var->name = name;
    return var;
}

Operand *new_label()
{
    Operand *var = (Operand *)malloc(sizeof(Operand));
    var->kind = LABEL_OP;
    var->var_no = lab_no;
    lab_no++;
    return var;
}

InterCodes *new_intercodes(Operand *op1, Operand *op2, Operand *op3, int type, char *relop)
{
    InterCodes *code = (InterCodes *)malloc(sizeof(InterCodes));
    code->prev = NULL;
    code->next = NULL;
    code->code = (InterCode *)malloc(sizeof(InterCode));
    code->code->kind = type;
    if (type == ASSIGN_IC ||
        type == RIGHT_STAR_IC ||
        type == LEFT_STAR_IC)
    {
        code->code->assign.left = op1;
        code->code->assign.right = op2;
    }
    else if (type == RETURN_IC)
    {
        code->code->return_.place = op1;
    }
    else if (type == FUN_IC)
    {
        code->code->func.func = op1;
    }
    else if (type == ADD_IC ||
             type == SUB_IC ||
             type == MUL_IC ||
             type == DIV_IC ||
             type == RIGHT_ADDR_IC)
    {
        code->code->binop.result = op1;
        code->code->binop.op1 = op2;
        code->code->binop.op2 = op3;
    }
    else if (type == READ_IC)
    {
        code->code->read_.readed = op1;
    }
    else if (type == LABEL_IC)
    {
        code->code->lab.label = op1;
    }
    else if (type == IF_IC)
    {
        code->code->if_goto.left = op1;
        code->code->if_goto.right = op2;
        code->code->if_goto.label = op3;
        code->code->if_goto.relop = relop;
    }
    else if (type == GOTO_IC)
    {
        code->code->goto_.label = op1;
    }
    else if (type == WRITE_IC)
    {
        code->code->write_.written = op1;
    }
    else if (type == CALL_IC)
    {
        code->code->call_.left = op1;
        code->code->call_.callee = relop;
    }
    else if (type == ARG_IC)
    {
        code->code->arg_.arg = op1;
    }
    else if (type == PARAM_IC)
    {
        code->code->param_.field = op1;
    }
    else if (type == DEC_IC)
    {
        code->code->dec_.var = op1;
        code->code->dec_.size = op1->size;
    }
    else
    {
        assert(0);
    }
    return code;
}

Operands *new_operands(Operand *op)
{
    Operands *ans = malloc(sizeof(Operands));
    ans->op = op;
    ans->next = NULL;
    ans->prev = NULL;
    return ans;
}