#include "table.h"
#include "hash.h"
#include "list.h"
#include "errorhandle.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

listnode_t *undefined_func = NULL;
hashtable_t symbol_hashtable;

table_t *init_table()
{
    hash_init(&symbol_hashtable);
    table_t *head = (table_t *)malloc(sizeof(table_t));
    head->pre = NULL;
    head->next = NULL;
    head->node = NULL;
    return head;
}

void semantic_analyse(node_t *root)
{
    init_table();
    Program(root);
}

/*
return value:
1: inconsistent declaration
*/
static int handle_func_declared(tnode_t *func_node)
{
    tnode_t *old_declared = hash_search(&symbol_hashtable, func_node->name, FUNC_DEC);
    tnode_t *old_defined = hash_search(&symbol_hashtable, func_node->name, FUNC);
    if (old_declared != NULL)
    {
        if (!is_func_same(func_node, old_declared))
            return 1;
    }
    if (old_defined != NULL)
    {
        if (!is_func_same(func_node, old_defined))
            return 1;
    }
    else
        list_insert(&undefined_func, func_node);
    hash_insert(&symbol_hashtable, func_node);
    return 0;
}

/*
reture value:
1: inconsistent definition
2: redefined function*/
static int handle_func_def(tnode_t *func_node)
{
    if (hash_search(&symbol_hashtable, func_node->name, FUNC) != NULL)
        return 2;

    tnode_t *old_declared = hash_search(&symbol_hashtable, func_node->name, FUNC_DEC);
    if (old_declared != NULL)
    {
        if (!is_func_same(func_node, old_declared))
            return 1;
        list_remove(&undefined_func, func_node->name);
    }
    hash_insert(&symbol_hashtable, func_node);
    return 0;
}

void handle_undef_func()
{
    while (undefined_func)
    {
        throw_error(18, undefined_func->tnode->line, undefined_func->tnode->name);
        undefined_func = undefined_func->next;
    }
}

static int is_func_same(tnode_t *a, tnode_t *b)
{
    func_t *af = a->func, *bf = b->func;
    if (af->returntype != bf->returntype)
        return 0;

    if (af->para == NULL && bf->para == NULL)
        return 1;
    else if (af->para == NULL || bf->para == NULL)
        return 0;

    FieldList_t *ap = af->para, *bp = bf->para;
    while (ap && bp)
    {
        if (ap->type->vartype != bp->type->vartype)
            return 0;
        ap = ap->tail;
        bp = bp->tail;
    }
    if (ap || bp)
        return 0;
    return 1;
}

static int is_array_equal(tnode_t *a, tnode_t *b)
{
    if (a->type != ARRAY || b->type != ARRAY)
        assert(0);

    if (a->array.demin != b->array.demin)
        return 0;
    while (a->type == ARRAY && b->type == ARRAY)
    {
        if (a->array.size != b->array.size)
            return 0;
        a = a->array.elem;
        b = b->array.elem;
    }
    if (a->type != b->type)
        return 0;
    return 1;
}

static int is_struct_equal(tnode_t *a, tnode_t *b)
{
    if (a->type != STRUCTUR || b->type != STRUCTUR)
        return 0;

    structure_t *as = a->structure, *bs = b->structure;
    FieldList_t *af = as->item, *bf = bs->item;

    while (af && bf)
    {
        if (af->type->type != bf->type->type)
            return 0;
        else if (af->type->type == STRUCTUR || af->type->type == STRUCTUR_USE)
        {
            if (!is_struct_equal(af->type, bf->type))
                return 0;
        }
        else if (af->type->type == ARRAY)
        {
            if (!is_array_equal(af->type, bf->type))
                return 0;
        }
        else if (af->type->type == VAR)
        {
            if (af->type->vartype != bf->type->vartype)
                return 0;
        }
        else
            assert(0);
        af = af->tail;
        bf = bf->tail;
    }
    if (af || bf)
        return 0;
    return 1;
}

void Program(node_t *node)
{
    if (node != NULL)
    {
        ExtDefList(node->child_nodes[0]);
    }
}

void ExtDefList(node_t *node)
{
    if (node != NULL)
    {
        ExtDef(node->child_nodes[0]);
        ExtDefList(node->child_nodes[1]); // may have problem
    }
}

void ExtDef(node_t *node)
{
    vtype functype = Specifier(node->child_nodes[0]);
    if (strcmp(node->child_nodes[1]->text, "FunDec") == 0)
    {
        hash_stack_increase(&symbol_hashtable);

        tnode_t *func_node = FunDec(node->child_nodes[1], functype);
        func_node->line = node->line;
        if (strcmp(node->child_nodes[2]->text, "SEMI") == 0)
        {
            func_node->type = FUNC_DEC;
            if (handle_func_declared(func_node) == 1)
                throw_error(19, node->line, func_node->name);
        }
        else
        {
            int retcode = handle_func_def(func_node);
            if (retcode == 1)
                throw_error(19, node->line, func_node->name);
            else if (retcode == 2)
                throw_error(4, node->line, func_node->name);
            CompSt(node->child_nodes[2], functype, 1);
        }

        hash_stack_decrease(&symbol_hashtable);
    }
}

tnode_t *FunDec(node_t *node, vtype type)
{
    tnode_t *insert_item = (tnode_t *)malloc(sizeof(tnode_t));
    insert_item->type = FUNC;
    insert_item->name = node->child_nodes[0]->text;
    func_t *func = (func_t *)malloc(sizeof(func_t));
    func->returntype = type;
    if (strcmp(node->child_nodes[2]->text, "VarList") == 0)
        func->para = VarList(node->child_nodes[2]);
    else
        func->para = NULL;
    insert_item->func = func;
    return insert_item;
}

FieldList_t *VarList(node_t *node)
{
    FieldList_t *para = (FieldList_t *)malloc(sizeof(FieldList_t));
    para->type = ParamDec(node->child_nodes[0]);
    if (node->child_nodes[1] != NULL)
        para->tail = VarList(node->child_nodes[2]);
    return para;
}

tnode_t *ParamDec(node_t *node)
{
    vtype type = Specifier(node->child_nodes[0]);
    tnode_t *var;
    var = VarDec(node->child_nodes[1], type, NULL);
    tnode_t *search_result = hash_search(&symbol_hashtable, var->name, 0);
    if (search_result != NULL)
    {
        if ((search_result->type != VAR || search_result->type != ARRAY) || search_result->nested_depth == symbol_hashtable.stack_frame)
            throw_error(3, node->line, var->name);
        else
            hash_insert(&symbol_hashtable, var);
    }
    else
        hash_insert(&symbol_hashtable, var);
    return var;
}

vtype Specifier(node_t *node)
{
    if (node->child_nodes[0]->type == TYPE_T)
    {
        if (strcmp(node->child_nodes[0]->text, "int") == 0)
            return VINT;
        else if (strcmp(node->child_nodes[0]->text, "float") == 0)
            return VFLOAT;
    }
    else if (strcmp(node->child_nodes[0]->text, "StructSpecifier") == 0)
    {
        return StructSpecifier(node->child_nodes[0]);
    }
}

vtype StructSpecifier(node_t *node)
{
    if (node->child_nodes[2] != NULL)
    {
        tnode_t *insert_item = (tnode_t *)malloc(sizeof(tnode_t));
        structure_t *struction = (structure_t *)malloc(sizeof(structure_t));
        insert_item->type = STRUCTUR;
        insert_item->structure = struction;
        if (strcmp(node->child_nodes[1]->text, "OptTag") == 0)
        {
            insert_item->name = node->child_nodes[1]->child_nodes[0]->text;
            for (int i = 2; i < 4; i++)
            {
                if (strcmp(node->child_nodes[i]->text, "DefList") == 0)
                {
                    hash_stack_increase(&symbol_hashtable);
                    struction->item = DefList(node->child_nodes[i], STRUCTUR);
                    hash_stack_decrease(&symbol_hashtable);
                    // struction->istype=1;
                }
            }
            tnode_t *search_result = hash_search(&symbol_hashtable, insert_item->name, 0);
            if (search_result != NULL && search_result->nested_depth == symbol_hashtable.stack_frame)
                throw_error(16, node->line, insert_item->name);
            else
                hash_insert(&symbol_hashtable, insert_item);
        }
        return STRUCTUR;
    }
    else if (strcmp(node->child_nodes[1]->text, "Tag") == 0)
        return STRUCTUR_USE;
    return STRUCTUR;
}

void CompSt(node_t *node, vtype functype, int is_func)
{
    if (!is_func)
        hash_stack_increase(&symbol_hashtable);
    for (int i = 1; (i < 3) && (node->child_nodes[i] != NULL); i++)
    {
        if (strcmp(node->child_nodes[i]->text, "DefList") == 0)
        {
            DefList(node->child_nodes[i], VAR); // 这里虽然输入VAR，但只是为了和STRUCTUR相区分
        }
        else if (strcmp(node->child_nodes[i]->text, "StmtList") == 0)
        {
            StmtList(node->child_nodes[i], functype);
        }
    }
    if (!is_func)
        hash_stack_decrease(&symbol_hashtable);
}

FieldList_t *DefList(node_t *node, ttype isstruc)
{ // 这里会将结构体中的域也保存到符号表中，最后实现变量的作用域时需要修改
    FieldList_t *para;
    FieldList_t *para_list;
    if (node != NULL)
    {
        if (isstruc == STRUCTUR)
        {
            para = Def(node->child_nodes[0], isstruc);
            FieldList_t *p;
            p = para;
            para_list = DefList(node->child_nodes[1], isstruc);
            while (p->tail != NULL)
                p = p->tail;
            p->tail = para_list;
        }
        else
        {
            Def(node->child_nodes[0], isstruc);
            DefList(node->child_nodes[1], isstruc);
        }
    }
    else
        return NULL;
    if (isstruc != STRUCTUR)
        return NULL;
    return para;
}

FieldList_t *Def(node_t *node, ttype isstruc)
{
    vtype type = Specifier(node->child_nodes[0]);
    char *struct_name = NULL;
    if (type == STRUCTUR_USE)
        struct_name = node->child_nodes[0]->child_nodes[0]->child_nodes[1]->child_nodes[0]->text;
    FieldList_t *para = DecList(node->child_nodes[1], type, isstruc, struct_name);
    if (isstruc != STRUCTUR)
        return NULL;
    return para;
}

FieldList_t *DecList(node_t *node, vtype type, ttype isstruc, char *struct_name)
{
    FieldList_t *para = Dec(node->child_nodes[0], type, isstruc, struct_name);
    FieldList_t *para_list;
    if (node->child_nodes[1] != NULL)
    {
        para_list = DecList(node->child_nodes[2], type, isstruc, struct_name);
        if (isstruc == STRUCTUR)
        {
            para->tail = para_list;
        }
    }
    if (isstruc != STRUCTUR)
        return NULL;
    return para;
}

FieldList_t *Dec(node_t *node, vtype type, ttype isstruc, char *struct_name)
{
    FieldList_t *para = (FieldList_t *)malloc(sizeof(FieldList_t));
    para->tail = NULL;
    tnode_t *var;
    var = VarDec(node->child_nodes[0], type, struct_name);
    para->type = var;
    tnode_t *search_node = hash_search(&symbol_hashtable, var->name, 0);
    if (search_node != NULL && search_node->nested_depth == symbol_hashtable.stack_frame)
    {
        if (isstruc == STRUCTUR)
            throw_error(15, node->line, var->name);
        else
            throw_error(3, node->line, var->name);
    }
    else
        hash_insert(&symbol_hashtable, var);

    if (node->child_nodes[1] != NULL)
    {
        if (Exp(node->child_nodes[2])->vartype != type)
            throw_error(5, node->line, NULL);
    }
    if (isstruc != STRUCTUR)
        return NULL;
    return para;
}

tnode_t *VarDec(node_t *node, vtype type, char *struct_name)
{
    tnode_t *var;
    var = NULL;
    if (node->child_nodes[0]->type == ID_T)
    {
        tnode_t *insert_item = (tnode_t *)malloc(sizeof(tnode_t));
        insert_item->type = VAR;
        insert_item->name = node->child_nodes[0]->text;
        if (struct_name != NULL)
        {
            insert_item->type = STRUCTUR; // TODO:find way to the use of struct
            tnode_t *search_result = hash_search(&symbol_hashtable, struct_name, STRUCTUR);
            if (search_result != NULL)
            {
                insert_item->structure = search_result->structure;
                insert_item->structure->struct_name = struct_name;
            }
            else
                throw_error(17, node->line, struct_name);
        }
        else
            insert_item->vartype = type;
        var = insert_item;
    }
    else
    {
        var = (tnode_t *)malloc(sizeof(tnode_t));
        var->type = ARRAY;
        var->array.elem = VarDec(node->child_nodes[0], type, struct_name);
        var->array.size = node->child_nodes[2]->value.int_val;
        var->name = var->array.elem->name;
        int demin = 1;
        tnode_t *temp = var;
        while (temp->array.elem->type == ARRAY)
        {
            temp = temp->array.elem;
            demin++;
        }
        var->array.demin = demin;
    }
    return var;
}

void StmtList(node_t *node, vtype functype)
{
    if (node != NULL)
    {
        Stmt(node->child_nodes[0], functype);
        StmtList(node->child_nodes[1], functype);
    }
}

void Stmt(node_t *node, vtype functype)
{
    if (strcmp(node->child_nodes[0]->text, "Exp") == 0)
        Exp(node->child_nodes[0]);
    else if (strcmp(node->child_nodes[0]->text, "RETURN") == 0)
    {
        if (Exp(node->child_nodes[1])->vartype != functype)
        {
            throw_error(8, node->child_nodes[1]->line, NULL);
        }
    }
    else if (strcmp(node->child_nodes[0]->text, "IF") == 0)
    {
        if (node->child_nodes[5] == NULL)
        {
            Exp(node->child_nodes[2]);
            Stmt(node->child_nodes[4], functype);
        }
        else
        {
            Exp(node->child_nodes[2]);
            Stmt(node->child_nodes[4], functype);
            Stmt(node->child_nodes[6], functype);
        }
    }
    else if (strcmp(node->child_nodes[0]->text, "WHILE") == 0)
    {
        Exp(node->child_nodes[2]);
        Stmt(node->child_nodes[4], functype);
    }
    else if (strcmp(node->child_nodes[0]->text, "CompSt") == 0)
        CompSt(node->child_nodes[0], functype, 0);
    else
        assert(0);
}

tnode_t *Exp(node_t *node)
{
    if (strcmp(node->child_nodes[0]->text, "Exp") == 0)
    {
        tnode_t *node1;
        node1 = Exp(node->child_nodes[0]);
        tnode_t *node2;
        vtype type1, type2;
        if ((strcmp(node->child_nodes[1]->text, "LB") != 0) && (strcmp(node->child_nodes[1]->text, "DOT") != 0))
        {
            node2 = Exp(node->child_nodes[2]);
            if (node1->type == VAR)
                type1 = node1->vartype;
            if (node2->type == VAR)
                type2 = node2->vartype;
            else if (node2->type == FUNC || node2->type == FUNC_DEC)
                type2 = node2->vartype;
        }

        if (strcmp(node->child_nodes[1]->text, "LB") == 0)
        { // 数组
            if (node->child_nodes[2]->child_nodes[0]->type == FLOAT_T)
            {
                throw_error(12, node->line, node->child_nodes[2]->child_nodes[0]->text);
                tnode_t *p;
                p = (tnode_t *)malloc(sizeof(tnode_t));
                p->vartype = VERROR;
                return p;
            }
            tnode_t *p = hash_search(&symbol_hashtable, node->child_nodes[0]->child_nodes[0]->text, ARRAY);
            if (p == NULL)
            { // 只有有定义才报此错，没有定义过则不报此错
                if (node->child_nodes[0]->child_nodes[0]->child_nodes[0] == NULL)
                { // 避免对数组多次报错
                    throw_error(10, node->line, node->child_nodes[0]->child_nodes[0]->text);
                }
                tnode_t *p;
                p = (tnode_t *)malloc(sizeof(tnode_t));
                p->vartype = VERROR;
                return p;
            }
            else
            {
                p = (tnode_t *)malloc(sizeof(tnode_t));
                p->type = ARRAY;
                return p;
            }
        }
        else if ((type1 == VERROR) || (type2 == VERROR))
        {
            node1->vartype = VERROR;
            return node1;
        }
        else if (strcmp(node->child_nodes[1]->text, "DOT") == 0)
        { // 结构体
            tnode_t *p = hash_search(&symbol_hashtable, node->child_nodes[0]->child_nodes[0]->text, STRUCTUR);
            char *now_field = node->child_nodes[2]->text;
            vtype filed_type = VERROR;
            if (p == NULL)
            {
                p = (tnode_t *)malloc(sizeof(tnode_t));
                p->vartype = VERROR;
                p->type = VAR;
                throw_error(13, node->line, NULL);
            }
            else
            {
                FieldList_t *origin_field = node1->structure->item;
                int find_field = 0;
                while (origin_field != NULL)
                {
                    if (strcmp(now_field, origin_field->type->name) == 0)
                    {
                        find_field = 1;
                        filed_type = origin_field->type->vartype;
                    }
                    origin_field = origin_field->tail;
                }
                if (find_field == 0)
                {
                    throw_error(14, node->line, now_field);
                    tnode_t *result;
                    result = (tnode_t *)malloc(sizeof(tnode_t));
                    result->vartype = VERROR;
                    result->type = VAR;
                    return result;
                }
                tnode_t *result;
                result = (tnode_t *)malloc(sizeof(tnode_t));
                result->vartype = filed_type;
                result->type = VAR;
                return result;
            }
            return p;
        }
        else if (strcmp(node->child_nodes[1]->text, "ASSIGNOP") == 0)
        {
            if ((node1->type == STRUCTUR) && (node1->type == STRUCTUR))
            { // 两个结构体赋值
                if (is_struct_equal(node1, node2))
                {
                    tnode_t *p;
                    p = (tnode_t *)malloc(sizeof(tnode_t));
                    p->vartype = VINT;
                    p->type = VAR;
                    return p;
                }
                else
                {
                    throw_error(5, node->line, NULL);
                    tnode_t *p = (tnode_t *)malloc(sizeof(tnode_t));
                    p->vartype = VERROR;
                    p->type = VAR;
                    return p;
                }
            }
            if ((node1->type == ARRAY))
            {
                tnode_t *p;
                p = (tnode_t *)malloc(sizeof(tnode_t));
                p->vartype = VINT;
                p->type = VAR;
                tnode_t *temp1 = node1;
                while (temp1->array.elem->type == ARRAY)
                {
                    temp1 = temp1->array.elem;
                }
                temp1 = temp1->array.elem;
                ttype leftype = temp1->type;
                ttype rightype;
                tnode_t *temp2 = node2;
                if (node2->type == ARRAY)
                {
                    while (temp2->array.elem->type == ARRAY)
                    {
                        temp2 = temp2->array.elem;
                    }
                    temp2 = temp2->array.elem;
                }
                rightype = temp2->type;
                if (rightype == leftype)
                {
                    if (rightype == VAR)
                    {
                        if ((temp1->vartype == temp2->vartype) && (node1->array.demin == node2->array.demin))
                            return p;
                        else
                            throw_error(5, node->line, NULL);
                    }
                    else if (rightype == FUNC)
                    {
                        if ((temp1->func->returntype == temp2->func->returntype) && (node1->array.demin == node2->array.demin))
                            return p;
                        else
                        {
                            throw_error(5, node->line, NULL);
                        }
                    }
                    else if (rightype == STRUCTUR)
                    {
                        if (!is_struct_equal(temp1, temp2))
                            return p;
                        else
                            throw_error(5, node->line, NULL);
                    }
                }
                return p;
            }
            else if (node->child_nodes[0]->child_nodes[0]->type != ID_T)
            {
                throw_error(6, node->line, NULL);
                tnode_t *p;
                p = (tnode_t *)malloc(sizeof(tnode_t));
                p->vartype = VERROR;
                p->type = VAR;
                return p;
            }
            else if (type1 != type2)
            {
                throw_error(5, node->line, NULL);
                tnode_t *p;
                p = (tnode_t *)malloc(sizeof(tnode_t));
                p->vartype = VERROR;
                p->type = VAR;
                return p;
            }
            else
            {
                tnode_t *p;
                p = (tnode_t *)malloc(sizeof(tnode_t));
                p->vartype = VINT;
                p->type = VAR;
                return p;
            }
        }
        else if (type1 == type2)
        {
            return node1;
        }
        else
        {
            throw_error(7, node->line, NULL);
            node1->vartype = VERROR;
            return node1;
        }
    }
    else if (node->child_nodes[0]->type == ID_T)
    {
        if (node->child_nodes[1] == NULL)
        { // 表示Exp->ID这种情况，可能是变量，也可能是结构体,也有可能是数组指针
            tnode_t *p1 = hash_search(&symbol_hashtable, node->child_nodes[0]->text, VAR);
            tnode_t *p2 = hash_search(&symbol_hashtable, node->child_nodes[0]->text, ARRAY);
            tnode_t *p3 = hash_search(&symbol_hashtable, node->child_nodes[0]->text, STRUCTUR);
            if ((p1 == NULL) && (p2 == NULL) && (p3 == NULL))
            {
                throw_error(1, node->line, node->child_nodes[0]->text);
                tnode_t *p;
                p = (tnode_t *)malloc(sizeof(tnode_t));
                p->vartype = VERROR;
                p->type = VAR;
                return p;
            }
            else if (p1 != NULL)
                return p1;
            else if (p2 != NULL)
                return p2;
            else if (p3 != NULL)
            { // 结构体的实例
                return p3;
            }
        }
        else
        { // 表示Exp推出函数这种情况
            tnode_t *search_result_fun = hash_search(&symbol_hashtable, node->child_nodes[0]->text, FUNC);
            if (search_result_fun == NULL)
                search_result_fun = hash_search(&symbol_hashtable, node->child_nodes[0]->text, FUNC_DEC);
            tnode_t *search_result_var = hash_search(&symbol_hashtable, node->child_nodes[0]->text, VAR);
            tnode_t *search_result_arr = hash_search(&symbol_hashtable, node->child_nodes[0]->text, ARRAY);
            if (search_result_fun == NULL)
            {
                if ((search_result_arr != NULL) || (search_result_var != NULL))
                    throw_error(11, node->line, node->child_nodes[0]->text);
                else
                    throw_error(2, node->line, node->child_nodes[0]->text);
                tnode_t *p;
                p = (tnode_t *)malloc(sizeof(tnode_t));
                p->vartype = VERROR;
                p->type = VAR;
                return p;
            }
            else
            {
                FieldList_t *new_para = NULL;
                if (node->child_nodes[3] != NULL)
                    new_para = Args(node->child_nodes[2]);
                FieldList_t *origin_para = search_result_fun->func->para;
                while ((new_para != NULL) || (origin_para != NULL))
                {
                    if ((origin_para == NULL) || (new_para == NULL))
                    {
                        throw_error(9, node->line, NULL);
                        tnode_t *p;
                        p = (tnode_t *)malloc(sizeof(tnode_t));
                        p->vartype = VERROR;
                        p->type = VAR;
                        return p;
                    }
                    else if (new_para->type->vartype != origin_para->type->vartype)
                    {
                        throw_error(9, node->line, NULL);
                        tnode_t *p;
                        p = (tnode_t *)malloc(sizeof(tnode_t));
                        p->vartype = VERROR;
                        p->type = VAR;
                        return p;
                    }
                    else
                    {
                        new_para = new_para->tail;
                        origin_para = origin_para->tail;
                    }
                }
                tnode_t *p;
                p = (tnode_t *)malloc(sizeof(tnode_t));
                p->vartype = search_result_fun->func->returntype;
                p->type = FUNC;
                return p;
            }
        }
    }
    else if (node->child_nodes[0]->type == INT_T)
    {
        tnode_t *p;
        p = (tnode_t *)malloc(sizeof(tnode_t));
        p->type = VAR;
        p->vartype = VINT;
        return p;
    }
    else if (node->child_nodes[0]->type == FLOAT_T)
    {
        tnode_t *p;
        p = (tnode_t *)malloc(sizeof(tnode_t));
        p->type = VAR;
        p->vartype = VFLOAT;
        return p;
    }
    printf("you can't reach here!!!!!!\n"); // 到达了这里意味着还有分类情况没有return正确的值或者没有return，有可能出现段错误
}

FieldList_t *Args(node_t *node)
{
    FieldList_t *para = (FieldList_t *)malloc(sizeof(FieldList_t));
    tnode_t *para_node = (tnode_t *)malloc(sizeof(tnode_t));
    para_node = Exp(node->child_nodes[0]);
    para->type = para_node;
    if (node->child_nodes[1] != NULL)
    {
        para->tail = Args(node->child_nodes[2]);
    }
    return para;
}