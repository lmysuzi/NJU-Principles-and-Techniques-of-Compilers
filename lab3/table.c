#include "table.h"
#include "hash.h"
#include "list.h"
#include "errorhandle.h"
// #include "translate.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

listnode_t *undefined_func = NULL;
hashtable_t symbol_hashtable;
int nametemp = 0; // 记录无名结构

static int is_array_equal(tnode_t *a, tnode_t *b);
static int is_struct_equal(tnode_t *a, tnode_t *b);

table_t *init_table()
{
    hash_init(&symbol_hashtable);
    table_t *head = (table_t *)malloc(sizeof(table_t));
    head->pre = NULL;
    head->next = NULL;
    head->node = NULL;
    return head;
}

static int handle_func_def(tnode_t *func_node);
void add_read_and_write()
{
    hash_stack_increase(&symbol_hashtable);

    tnode_t *read_func = (tnode_t *)malloc(sizeof(tnode_t));
    read_func->type = FUNC;
    read_func->name = "read";
    func_t *read_f = (func_t *)malloc(sizeof(func_t));
    read_f->returntype = VINT;
    read_f->para = NULL;
    read_func->func = read_f;

    tnode_t *write_func = (tnode_t *)malloc(sizeof(tnode_t));
    write_func->type = FUNC;
    write_func->name = "write";
    func_t *write_f = (func_t *)malloc(sizeof(func_t));
    write_f->returntype = VINT;
    FieldList_t *para = (FieldList_t *)malloc(sizeof(FieldList_t));
    para->tail = NULL;
    tnode_t *int_para = (tnode_t *)malloc(sizeof(tnode_t));
    int_para->type = VAR;
    int_para->vartype = VINT;
    para->type = int_para;
    write_f->para = para;
    write_func->func = write_f;

    handle_func_def(read_func);
    handle_func_def(write_func);

    hash_stack_decrease(&symbol_hashtable);
}

void semantic_analyse(node_t *root)
{
    init_table();
    add_read_and_write();
    Program(root);
    handle_undef_func();
}

static int is_para_same(FieldList_t *ap, FieldList_t *bp)
{
    while (ap && bp)
    {
        if (ap->type->type != bp->type->type)
        {
            return 0;
        }
        if (ap->type->type == ARRAY)
        {
            if (!is_array_equal(ap->type, bp->type))
                return 0;
        }
        else if (ap->type->type == STRUCTUR_USE)
        {
            if (!is_struct_equal(ap->type, bp->type))
                return 0;
        }
        else if (ap->type->type == VAR)
        {
            vtype at = ap->type->vartype;
            vtype bt = bp->type->vartype;
            if (at == VINT_VAR)
                at = VINT;
            else if (at == VFLOAT_VAR)
                at = VFLOAT;
            if (bt == VINT_VAR)
                bt = VINT;
            else if (bt == VFLOAT_VAR)
                bt = VFLOAT;
            if (at != bt)
                return 0;
        }
        ap = ap->tail;
        bp = bp->tail;
    }
    if (ap || bp)
        return 0;
    return 1;
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
    return is_para_same(ap, bp);
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

static int is_array_equal(tnode_t *a, tnode_t *b)
{
    if (a->type != ARRAY || b->type != ARRAY)
        assert(0);

    if (a->array.demin != b->array.demin)
        return 0;
    while (a->type == ARRAY && b->type == ARRAY)
    {
        a = a->array.elem;
        b = b->array.elem;
    }
    if (a->type != b->type)
        return 0;
    if (a->vartype != b->vartype)
        return 0;
    return 1;
}

static int is_struct_equal(tnode_t *a, tnode_t *b)
{
    if (a->type != STRUCTUR_USE || b->type != STRUCTUR_USE)
        return 0;

    structure_t *as = a->structure, *bs = b->structure;
    FieldList_t *af = as->item, *bf = bs->item;

    while (af && bf)
    {
        if (af->type->type != bf->type->type)
            return 0;
        else if (af->type->type == STRUCTUR_USE)
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
        if (functype == VINT_VAR)
            functype = VINT;
        else if (functype == VFLOAT_VAR)
            functype = VFLOAT;
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
    else if (strcmp(node->child_nodes[1]->text, "SEMI") == 0)
    {
        // did not need to do anything here
    }
    else if (strcmp(node->child_nodes[1]->text, "ExtDecList") == 0)
    {
        char *struct_name = NULL;
        if (functype == STRUCTUR)
        {
            if (strcmp(node->child_nodes[0]->child_nodes[0]->child_nodes[1]->text, "LC") == 0)
            {
                char *anoname = (char *)malloc(20 * sizeof(char));
                sprintf(anoname, "noname%d", nametemp - 1);
                struct_name = anoname;
            }
            else
            {
                struct_name = node->child_nodes[0]->child_nodes[0]->child_nodes[1]->child_nodes[0]->text;
            }
        }
        ExtDecList(node->child_nodes[1], functype, struct_name);
    }
}

void ExtDecList(node_t *node, vtype type, char *struct_name)
{
    tnode_t *var;
    int size = 0;
    if (struct_name != NULL)
    {
        tnode_t *struct_node = hash_search(&symbol_hashtable, struct_name, STRUCTUR);
        assert(struct_node != NULL);
        size = struct_node->size;
    }
    else
    {
        if (type == VINT || type == VFLOAT || type == VINT_VAR || type == VFLOAT_VAR)
            size = 4;
        else
            assert(0);
    }
    var = VarDec(node->child_nodes[0], type, struct_name, 0, size);
    if (var->type == ARRAY)
    {
        if (struct_name != NULL)
        {
            var->type = STRUCTUR_USE;
            var->vartype = VSTRUCT_VAR;
        }
        else
        {
            var->type = VAR;
            var->vartype = type;
        }
        int demension = 1;

        while (var->array.prev != NULL)
        {
            var = var->array.prev;
            var->array.demin = demension;
            demension++;
        }
    }

    tnode_t *search_node = hash_search(&symbol_hashtable, var->name, 0);
    if (search_node != NULL && (search_node->nested_depth == symbol_hashtable.stack_frame || search_node->nested_depth == -1))
    {
        if (type == STRUCTUR)
            throw_error(15, node->line, var->name);
        else
            throw_error(3, node->line, var->name);
    }
    else
    {
        hash_insert(&symbol_hashtable, var);
    }
    if (node->child_nodes[1] != NULL)
    {
        ExtDecList(node->child_nodes[2], type, struct_name);
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
    else
        para->tail = NULL;
    return para;
}

tnode_t *ParamDec(node_t *node)
{
    vtype type = Specifier(node->child_nodes[0]);
    char *struct_name = NULL;
    if (type == STRUCTUR_USE)
    {
        struct_name = node->child_nodes[0]->child_nodes[0]->child_nodes[1]->child_nodes[0]->text;
    }
    else if (type == STRUCTUR)
    {
        if (strcmp(node->child_nodes[0]->child_nodes[0]->child_nodes[1]->text, "LC") == 0)
        {
            char *anoname = (char *)malloc(20 * sizeof(char));
            sprintf(anoname, "noname%d", nametemp - 1);
            struct_name = anoname;
        }
        else
        {
            struct_name = node->child_nodes[0]->child_nodes[0]->child_nodes[1]->child_nodes[0]->text;
        }
    }
    tnode_t *var;
    int size = 0;
    if (struct_name != NULL)
    {
        tnode_t *struct_node = hash_search(&symbol_hashtable, struct_name, STRUCTUR);
        assert(struct_node != NULL);
        size = struct_node->size;
    }
    else
    {
        if (type == VINT || type == VFLOAT || type == VINT_VAR || type == VFLOAT_VAR)
            size = 4;
        else
            assert(0);
    }
    var = VarDec(node->child_nodes[1], type, struct_name, 0, size);
    if (var->type == ARRAY)
    {
        if (struct_name != NULL)
        {
            var->type = STRUCTUR_USE;
            var->vartype = VSTRUCT_VAR;
        }
        else
        {
            var->type = VAR;
            var->vartype = type;
        }
        int demension = 1;

        while (var->array.prev != NULL)
        {
            var = var->array.prev;
            var->array.demin = demension;
            demension++;
        }
    }

    tnode_t *search_result = hash_search(&symbol_hashtable, var->name, 0);
    if (search_result != NULL)
    {
        if (search_result->nested_depth == -1 || search_result->nested_depth == symbol_hashtable.stack_frame)
            throw_error(3, node->line, var->name);
        else
            hash_insert(&symbol_hashtable, var);
    }
    else
        hash_insert(&symbol_hashtable, var);

    var->is_param = IS_PARAM;
    return var;
}

vtype Specifier(node_t *node)
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
            hash_stack_increase(&symbol_hashtable);
            if (strcmp(node->child_nodes[3]->text, "DefList") == 0)
            {
                struction->item = DefList(node->child_nodes[3], STRUCTUR);
                int size = 0;
                FieldList_t *field = struction->item;
                while (field != NULL)
                {
                    field->offset = size;
                    size += field->type->size;
                    field = field->tail;
                }
                insert_item->size = size;
            }
            else
                struction->item = NULL;
            hash_stack_decrease(&symbol_hashtable);
            // struction->istype=1;
            tnode_t *search_result = hash_search(&symbol_hashtable, insert_item->name, 0);
            if (search_result != NULL && (search_result->nested_depth == symbol_hashtable.stack_frame || search_result->nested_depth == -1))
                throw_error(16, node->line, insert_item->name);
            else
                hash_insert(&symbol_hashtable, insert_item);
        }
        else
        {
            char *anoname = (char *)malloc(20 * sizeof(char));
            sprintf(anoname, "noname%d", nametemp);
            insert_item->name = anoname;
            nametemp++;
            for (int i = 2; i <= 2; i++)
            {
                if (strcmp(node->child_nodes[i]->text, "DefList") == 0)
                {
                    hash_stack_increase(&symbol_hashtable);
                    struction->item = DefList(node->child_nodes[i], STRUCTUR);
                    hash_stack_decrease(&symbol_hashtable);
                    // struction->istype=1;
                    int size = 0;
                    FieldList_t *field = struction->item;
                    while (field != NULL)
                    {
                        field->offset = size;
                        size += field->type->size;
                        field = field->tail;
                    }
                    insert_item->size = size;
                }
            }
            tnode_t *search_result = hash_search(&symbol_hashtable, insert_item->name, 0);
            if (search_result != NULL && (search_result->nested_depth == symbol_hashtable.stack_frame || search_result->nested_depth == -1))
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
    {
        struct_name = node->child_nodes[0]->child_nodes[0]->child_nodes[1]->child_nodes[0]->text;
    }
    else if (type == STRUCTUR)
    {
        if (strcmp(node->child_nodes[0]->child_nodes[0]->child_nodes[1]->text, "LC") == 0)
        {
            char *anoname = (char *)malloc(20 * sizeof(char));
            sprintf(anoname, "noname%d", nametemp - 1);
            struct_name = anoname;
        }
        else
        {
            struct_name = node->child_nodes[0]->child_nodes[0]->child_nodes[1]->child_nodes[0]->text;
        }
    }
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

    int size = 0;
    if (struct_name != NULL)
    {
        tnode_t *struct_node = hash_search(&symbol_hashtable, struct_name, STRUCTUR);
        assert(struct_node != NULL);
        size = struct_node->size;
    }
    else
    {
        if (type == VINT || type == VFLOAT || type == VINT_VAR || type == VFLOAT_VAR)
            size = 4;
        else
            assert(0);
    }

    var = VarDec(node->child_nodes[0], type, struct_name, 0, size);

    if (var->type == ARRAY)
    {
        if (struct_name != NULL)
        {
            var->type = STRUCTUR_USE;
            var->vartype = VSTRUCT_VAR;
        }
        else
        {
            var->type = VAR;
            var->vartype = type;
        }
        int demension = 1;

        while (var->array.prev != NULL)
        {
            var = var->array.prev;
            var->array.demin = demension;
            demension++;
        }
    }
    para->type = var;
    tnode_t *search_node = hash_search(&symbol_hashtable, var->name, 0);

    if (isstruc == STRUCTUR)
    {
        if (node->child_nodes[1] != NULL)
        {
            throw_error(15, node->line, var->name);
        }
    }
    if (search_node != NULL && (search_node->nested_depth == symbol_hashtable.stack_frame || search_node->nested_depth == -1))
    {
        if (isstruc == STRUCTUR)
            throw_error(15, node->line, var->name);
        else
            throw_error(3, node->line, var->name);
    }
    else
    {
        hash_insert(&symbol_hashtable, var);
    }

    if (node->child_nodes[1] != NULL)
    {
        exp_t exp = Exp(node->child_nodes[2]);
        if (exp.type == VINT)
            exp.type = VINT_VAR;
        else if (exp.type == VFLOAT)
            exp.type = VFLOAT_VAR;
        if (exp.type != type)
            throw_error(5, node->line, NULL);
    }
    if (isstruc != STRUCTUR)
        return NULL;
    return para;
}

tnode_t *VarDec(node_t *node, vtype type, char *struct_name, int count, int size)
{
    // count为0时，说明不是数组，其类型应该为type；否则其类型为array
    tnode_t *var;
    var = NULL;
    if (node->child_nodes[0]->type == ID_T)
    {
        tnode_t *insert_item = (tnode_t *)malloc(sizeof(tnode_t));
        if (count == 0)
        {
            insert_item->size = size;
            if (struct_name == NULL)
                insert_item->type = VAR;
            else
                insert_item->type = STRUCTUR_USE;
        }
        else
        {
            insert_item->type = ARRAY;
            insert_item->size = count * size;
            insert_item->array.count = 1;
            insert_item->array.prev = insert_item->array.elem = NULL;
        }
        insert_item->name = node->child_nodes[0]->text;
        if (struct_name != NULL)
        {
            tnode_t *search_result = hash_search(&symbol_hashtable, struct_name, STRUCTUR);
            if (search_result != NULL)
            {
                insert_item->structure = search_result->structure;
                insert_item->structure->struct_name = struct_name;
                insert_item->vartype = VSTRUCT_VAR;
            }
            else
                throw_error(17, node->line, struct_name);
        }
        else
        {
            insert_item->vartype = type;
        }
        var = insert_item;
    }
    else
    {
        var = (tnode_t *)malloc(sizeof(tnode_t));
        var->array.elem = NULL;
        var->array.prev = NULL;
        var->type = ARRAY;
        var->array.count = node->child_nodes[2]->value.int_val;
        if (count == 0)
        {
            var->vartype = type;
            var->size = size;
        }
        else
        {
            var->size = count * size;
        }
        tnode_t *parent = VarDec(node->child_nodes[0], type, NULL, var->array.count, var->size);

        var->name = parent->name;
        parent->array.elem = var;
        var->array.prev = parent;
        if (struct_name != NULL)
        {
            tnode_t *search_result = hash_search(&symbol_hashtable, struct_name, STRUCTUR);
            if (search_result != NULL)
            {
                var->structure = search_result->structure;
                var->structure->struct_name = struct_name;
                var->vartype = VSTRUCT_VAR;
            }
            else
                throw_error(17, node->line, struct_name);
        }
        /*int demin = 1;
        tnode_t *temp = var;
        while (temp->array.elem->type == ARRAY)
        {
            temp = temp->array.elem;
            demin++;
        }
        var->array.demin = demin;*/
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
        exp_t e = Exp(node->child_nodes[1]);
        if (e.type == VINT_VAR)
            e.type = VINT;
        else if (e.type == VFLOAT_VAR)
            e.type = VFLOAT;
        if (e.type != functype)
        {
            throw_error(8, node->child_nodes[1]->line, NULL);
        }
    }
    else if (strcmp(node->child_nodes[0]->text, "IF") == 0)
    {
        if (node->child_nodes[5] == NULL)
        {
            exp_t e = Exp(node->child_nodes[2]);
            if (e.type != VINT && e.type != VINT_VAR)
                throw_error(7, node->line, NULL);
            Stmt(node->child_nodes[4], functype);
        }
        else
        {
            exp_t e = Exp(node->child_nodes[2]);
            if (e.type != VINT && e.type != VINT_VAR)
                throw_error(7, node->line, NULL);
            Stmt(node->child_nodes[4], functype);
            Stmt(node->child_nodes[6], functype);
        }
    }
    else if (strcmp(node->child_nodes[0]->text, "WHILE") == 0)
    {
        exp_t e = Exp(node->child_nodes[2]);
        if (e.type != VINT && e.type != VINT_VAR)
            throw_error(7, node->line, NULL);
        Stmt(node->child_nodes[4], functype);
    }
    else if (strcmp(node->child_nodes[0]->text, "CompSt") == 0)
        CompSt(node->child_nodes[0], functype, 0);
    else
        assert(0);
}

static exp_t binary_Exp(node_t *node)
{
    exp_t ret = {VERROR, NULL};
    exp_t operand1 = Exp(node->child_nodes[0]);
    exp_t operand2 = Exp(node->child_nodes[2]);
    int is_assign = (strcmp(node->child_nodes[1]->text, "ASSIGNOP") == 0);

    // error
    if (operand1.type == VERROR || operand2.type == VERROR)
        return ret;
    if (operand1.type == VARRAY || operand2.type == VARRAY)
    {
        throw_error(7, node->line, NULL);
        return ret;
    }
    if (operand1.type == VSTRUCT_VAR && operand2.type == VSTRUCT_VAR && is_assign)
    {
        if (!is_struct_equal(operand1.symbol, operand2.symbol))
        {
            throw_error(5, node->line, NULL);
            return ret;
        }
        ret.type = VINT;
        return ret;
    }
    else if (operand1.type == VSTRUCT_VAR || operand2.type == VSTRUCT_VAR)
    {
        if (is_assign)
            throw_error(5, node->line, NULL);
        else
            throw_error(7, node->line, NULL);
        return ret;
    }

    if ((operand1.type == VINT || operand1.type == VINT_VAR) && (operand2.type == VFLOAT || operand2.type == VFLOAT_VAR))
    {
        if (is_assign)
            throw_error(5, node->line, NULL);
        else
            throw_error(7, node->line, NULL);
        return ret;
    }

    if ((operand2.type == VINT || operand2.type == VINT_VAR) && (operand1.type == VFLOAT || operand1.type == VFLOAT_VAR))
    {
        if (is_assign)
            throw_error(5, node->line, NULL);
        else
            throw_error(7, node->line, NULL);
        return ret;
    }

    if (is_assign)
    {
        if (operand1.type == VINT || operand1.type == VFLOAT)
        {
            throw_error(6, node->line, NULL);
            return ret;
        }
    }
    else if (strcmp(node->child_nodes[1]->text, "AND") == 0 || strcmp(node->child_nodes[1]->text, "OR") == 0)
    {
        if (operand1.type == VFLOAT || operand1.type == VFLOAT_VAR || operand2.type == VFLOAT || operand2.type == VFLOAT_VAR)
        {
            throw_error(7, node->line, NULL);
            return ret;
        }
    }
    else if (strcmp(node->child_nodes[1]->text, "RELOP") == 0)
    {
    }

    if (operand1.type == VINT || operand1.type == VINT_VAR)
        ret.type = VINT;
    else if (operand1.type == VFLOAT || operand1.type == VFLOAT_VAR)
        ret.type = VFLOAT;
    else
        assert(0);
    if (is_assign || strcmp(node->child_nodes[1]->text, "RELOP") == 0)
        ret.type = VINT;
    return ret;
}

static exp_t assign_Exp(node_t *node)
{
    exp_t ret = {VERROR, NULL};
    if (node->child_nodes[0]->type == INT_T)
        ret.type = VINT;
    else if (node->child_nodes[0]->type == FLOAT_T)
        ret.type = VFLOAT;
    else if (node->child_nodes[0]->type == ID_T)
    {
        char *var_name = node->child_nodes[0]->text;
        tnode_t *var = hash_search(&symbol_hashtable, var_name, 0);

        if (var == NULL)
        {
            throw_error(1, node->line, var_name);
        }
        else if (var->type == STRUCTUR_USE)
        {
            ret.type = VSTRUCT_VAR;
            ret.symbol = var;
        }
        else if (var->type == VAR)
        {
            ret.type = var->vartype;
            ret.symbol = var;
        }
        else if (var->type == ARRAY)
        {
            ret.type = VARRAY;
            ret.symbol = var;
        }
        else
        {
            throw_error(5, node->line, var_name);
        }
    }
    else
        assert(0);

    return ret;
}

static exp_t func_Exp(node_t *node)
{
    exp_t ret = {VERROR, NULL};

    char *func_name = node->child_nodes[0]->text;
    tnode_t *func = hash_search(&symbol_hashtable, func_name, 0);

    if (func == NULL)
        throw_error(2, node->line, func_name);
    else if (func->type != FUNC && func->type != FUNC_DEC)
        throw_error(11, node->line, func_name);
    else
    {
        // no args
        if (func->func->para == NULL && strcmp(node->child_nodes[2]->text, "RP") == 0)
            ret.type = func->func->returntype;
        // args
        else if (func->func->para != NULL && strcmp(node->child_nodes[2]->text, "Args") == 0)
        {
            FieldList_t *actual_para = Args(node->child_nodes[2]);
            FieldList_t *formal_para = func->func->para;
            if (!is_para_same(actual_para, formal_para))
            {
                throw_error(9, node->line, func_name);
            }
            else
                ret.type = func->func->returntype;
        }
        else
            throw_error(9, node->line, func_name);
    }
    return ret;
}

static exp_t unary_Exp(node_t *node)
{
    exp_t ret = {VERROR, NULL};
    exp_t e = Exp(node->child_nodes[1]);
    if (e.type == VERROR)
        return ret;
    if (e.type == VARRAY || e.type == VSTRUCT_VAR)
    {
        throw_error(7, node->line, NULL);
        return ret;
    }

    if (e.type == VFLOAT || e.type == VFLOAT_VAR)
    {
        if (strcmp(node->child_nodes[0]->text, "NOT") == 0)
        {
            throw_error(7, node->line, NULL);
            return ret;
        }
    }
    if (e.type == VINT || e.type == VINT_VAR)
        ret.type = VINT;
    else if (e.type == VFLOAT || e.type == VFLOAT_VAR)
        ret.type = VFLOAT;
    return ret;
}

static exp_t dot_Exp(node_t *node)
{
    exp_t ret = {VERROR, NULL};
    exp_t struct_use = Exp(node->child_nodes[0]);

    if (struct_use.type != VSTRUCT_VAR)
        throw_error(13, node->line, NULL);
    else if (struct_use.symbol == NULL)
        assert(0);
    else
    {
        if (struct_use.symbol->type != STRUCTUR_USE)
            assert(0);

        char *field_name = node->child_nodes[2]->text;
        FieldList_t *struct_field = struct_use.symbol->structure->item;

        while (struct_field != NULL)
        {
            tnode_t *field_node = struct_field->type;
            if (strcmp(field_node->name, field_name) == 0)
            {
                if (field_node->type == STRUCTUR_USE)
                {
                    ret.type = VSTRUCT_VAR;
                    ret.symbol = field_node;
                }
                else if (field_node->type == VAR)
                {
                    // may have problem
                    ret.type = field_node->vartype;
                    ret.symbol = field_node;
                }
                else if (field_node->type == ARRAY)
                {
                    ret.type = VARRAY;
                    ret.symbol = field_node;
                }
                else
                    assert(0);
                break;
            }
            struct_field = struct_field->tail;
        }
        if (struct_field == NULL)
            throw_error(14, node->line, field_name);
    }
    return ret;
}

static exp_t array_Exp(node_t *node)
{
    exp_t ret = {VERROR, NULL};
    exp_t array = Exp(node->child_nodes[0]);
    exp_t index = Exp(node->child_nodes[2]);

    if (array.type != VERROR)
    {
        if (array.type != VARRAY)
        {
            throw_error(10, node->line, NULL);
        }
        else if (index.type != VERROR)
        {
            if (index.type != VINT && index.type != VINT_VAR)
            {
                // may have problem
                throw_error(12, node->line, NULL);
            }
            else
            {
                if (array.symbol->array.elem->type == ARRAY)
                {
                    ret.type = VARRAY;
                    ret.symbol = array.symbol->array.elem;
                }
                else if (array.symbol->array.elem->type == VAR)
                {
                    ret.type = array.symbol->array.elem->vartype;
                    ret.symbol = array.symbol->array.elem;
                }
                else if (array.symbol->array.elem->type == STRUCTUR_USE)
                {
                    ret.type = VSTRUCT_VAR;
                    ret.symbol = array.symbol->array.elem;
                }
                else
                {
                    assert(0);
                }
            }
        }
    }
    return ret;
}

exp_t Exp(node_t *node)
{
    exp_t ret = {VERROR, NULL};
    if (node->child_nodes[1] == NULL)
    {
        /*
        Exp → ID
            | INT
            | FLOAT
        */
        ret = assign_Exp(node);
    }
    else if (strcmp(node->child_nodes[0]->text, "Exp") == 0 && strcmp(node->child_nodes[2]->text, "Exp") == 0)
    {
        if (strcmp(node->child_nodes[1]->text, "LB") == 0)
        {
            /*
            Exp → Exp LB Exp RB
            */
            ret = array_Exp(node);
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
            ret = binary_Exp(node);
        }
    }
    else if (node->child_nodes[0]->type == ID_T)
    {
        // function
        /*
        Exp → ID LP Args RP
            | ID LP RP
        */
        ret = func_Exp(node);
    }
    else if (strcmp(node->child_nodes[0]->text, "LP") == 0)
    {
        /*
        Exp → LP Exp RP
        */
        ret = Exp(node->child_nodes[1]);
    }
    else if (strcmp(node->child_nodes[1]->text, "Exp") == 0)
    {
        /*
        Exp → MINUS Exp
            | NOT Exp
        */
        ret = unary_Exp(node);
    }
    else if (strcmp(node->child_nodes[1]->text, "DOT") == 0)
    {
        /*
        Exp → Exp DOT ID
        */
        ret = dot_Exp(node);
    }
    else
        assert(0);

    return ret;
}

FieldList_t *Args(node_t *node)
{
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
    else
        para->tail = NULL;
    return para;
}