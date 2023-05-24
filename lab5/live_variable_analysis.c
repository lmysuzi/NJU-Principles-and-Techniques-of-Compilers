#include "live_variable_analysis.h"

static int var_equal(void *a, void *b)
{
    if (a == b)
        return 1;
    return 0;
}

static void print_fact(CFGnode *cfgnode)
{
    Set *infact = cfgnode->in_fact;

    printf("%d out: ", cfgnode->stmt->index);
    Set *outfact = cfgnode->out_fact;
    for (ListNode *cur = outfact->head->next; cur != outfact->head; cur = cur->next)
    {
        Operand *op = (Operand *)cur->data;
        printf("%s ", op->name);
    }
    printf("\n%d in: ", cfgnode->stmt->index);
    for (ListNode *cur = infact->head->next; cur != infact->head; cur = cur->next)
    {
        Operand *op = (Operand *)cur->data;
        printf("%s ", op->name);
    }
    printf("\n");
}

static void init(const CFG *cfg)
{
    cfg->entry_node->in_fact = set_create();
    cfg->entry_node->out_fact = set_create();
    cfg->exit_node->in_fact = set_create();
    cfg->exit_node->out_fact = set_create();

    for (ListNode *cur = cfg->cfgnode_list->next; cur != cfg->cfgnode_list; cur = cur->next)
    {
        CFGnode *cfgnode = (CFGnode *)cur->data;
        cfgnode->in_fact = set_create();
        cfgnode->out_fact = set_create();
    }
}

static int transfer_node(CFGnode *cfgnode)
{
    if (cfgnode->type == EXIT)
        return 1;
    if (cfgnode->type == ENTRY)
        return 0;

    // 计算outfact
    Set *outfact = set_create();

    for (ListNode *succ_node = cfgnode->successors->next; succ_node != cfgnode->successors; succ_node = succ_node->next)
    {
        CFGnode *succ = (CFGnode *)succ_node->data;
        Set *new_outfact = set_union(outfact, succ->in_fact, var_equal);
        outfact = set_teardown(outfact);
        outfact = new_outfact;
    }

    // 复制outfact
    Set *infact = set_copy(outfact);

    // 计算def
    Operand *def = NULL;
    IR *stmt = cfgnode->stmt;

    if (stmt->type == ASSIGN_IR)
        def = stmt->assign_ir.left;
    else if (stmt->type == BINARY_IR)
        def = stmt->binary_ir.left;
    else if (stmt->type == CALL_IR)
        def = stmt->call_ir.left;
    else if (stmt->type == READ_IR)
        def = stmt->read_ir.read_var;

    if (def != NULL)
    {
        infact = set_sub_data(infact, def, var_equal);
    }

    // 计算use

    Operand *use1 = NULL;
    Operand *use2 = NULL;

    if (stmt->type == ASSIGN_IR)
    {
        use1 = stmt->assign_ir.right;
    }
    else if (stmt->type == BINARY_IR)
    {
        use1 = stmt->binary_ir.right1;
        use2 = stmt->binary_ir.right2;
    }
    else if (stmt->type == CONDITIONAL_GOTO_IR)
    {
        use1 = stmt->conditional_goto_ir.left;
        use2 = stmt->conditional_goto_ir.right;
    }
    else if (stmt->type == RETURN_IR)
    {
        use1 = stmt->return_ir.return_var;
    }
    else if (stmt->type == ARG_IR)
    {
        use1 = stmt->arg_ir.arg_var;
    }
    else if (stmt->type == WRITE_IR)
    {
        use1 = stmt->write_ir.write_var;
    }

    if (use1 != NULL && use1->type == VAR_OP)
        infact = set_add(infact, use1);
    if (use2 != NULL && use2->type == VAR_OP)
        infact = set_add(infact, use2);

    // 处理结果
    int changed = set_equal(infact, cfgnode->in_fact) == 1 ? 0 : 1;
    cfgnode->out_fact = set_teardown(cfgnode->out_fact);
    cfgnode->out_fact = outfact;
    cfgnode->in_fact = set_teardown(cfgnode->in_fact);
    cfgnode->in_fact = infact;

    return changed;
}

static void solver(const CFG *cfg)
{
    Queue *worklist = queue_create();

    // 将除entry以外的所有结点加入队列
    queue_push(worklist, cfg->exit_node);

    for (ListNode *cur = cfg->cfgnode_list->prev; cur != cfg->cfgnode_list; cur = cur->prev)
    {
        CFGnode *cfgnode = (CFGnode *)cur->data;
        queue_push(worklist, cfgnode);
    }

    while (!queue_empty(worklist))
    {
        CFGnode *cfgnode = (CFGnode *)queue_pop(worklist);

        if (transfer_node(cfgnode))
        {

            for (ListNode *pre = cfgnode->predecessors->next; pre != cfgnode->predecessors; pre = pre->next)
            {
                CFGnode *pre_cfgnode = (CFGnode *)pre->data;
                if (!queue_contains(worklist, pre_cfgnode))
                {
                    queue_push(worklist, pre_cfgnode);
                }
            }
        }
    }

    queue_teardown(worklist);
}

static void fact_teardown(CFG *cfg)
{
    cfg->entry_node->in_fact = set_teardown(cfg->entry_node->in_fact);
    cfg->entry_node->out_fact = set_teardown(cfg->entry_node->out_fact);
    cfg->exit_node->in_fact = set_teardown(cfg->exit_node->in_fact);
    cfg->exit_node->out_fact = set_teardown(cfg->exit_node->out_fact);
    for (ListNode *cur = cfg->cfgnode_list->next; cur != cfg->cfgnode_list; cur = cur->next)
    {
        CFGnode *cfgnode = (CFGnode *)cur->data;
        cfgnode->in_fact = set_teardown(cfgnode->in_fact);
        cfgnode->out_fact = set_teardown(cfgnode->out_fact);
    }
}

static void dead_code_elimination(CFG *cfg)
{
    for (ListNode *cur = cfg->cfgnode_list->next; cur != cfg->cfgnode_list; cur = cur->next)
    {
        CFGnode *cfgnode = (CFGnode *)cur->data;

        IR *stmt = cfgnode->stmt;
        Operand *left = NULL;

        if (stmt->type == ASSIGN_IR)
        {
            left = stmt->assign_ir.left;
        }
        else if (stmt->type == BINARY_IR)
        {
            left = stmt->binary_ir.left;
        }

        if (left != NULL && left->type == VAR_OP)
        {
            // outfact不存在赋值的变量，说明该变量在后文不活跃，该代码是死代码
            if (set_contains(cfgnode->out_fact, left, var_equal) == NULL)
                cfgnode->dead = 1;
        }
    }
}

void live_variable_analysis()
{
    for (ListNode *cfg_node = cfgs_list_head->next; cfg_node != cfgs_list_head; cfg_node = cfg_node->next)
    {
        CFG *cfg = (CFG *)cfg_node->data;
        init(cfg);
        solver(cfg);
        dead_code_elimination(cfg);
        // print_cfg_fact(cfg);

        fact_teardown(cfg);
    }
}