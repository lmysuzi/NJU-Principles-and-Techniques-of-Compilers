#include "avilable_expressions_analysis.h"

static void print_cfg_fact(CFG *cfg);

static void print_fact(CFGnode *cfgnode);

/*static ExpFact *exp_fact_create(Exp *exp, Operand *op)
{
    assert(exp != NULL);
    ExpFact *expfact = (ExpFact *)malloc(sizeof(ExpFact));
    assert(expfact != NULL);
    expfact->exp = exp;
    expfact->op = op;
    return expfact;
}*/

static int exp_equal(void *a, void *b)
{
    assert(a != NULL && b != NULL);
    if (a == b)
        return 1;
    return 0;
}

static void init(const CFG *cfg)
{
    ListNode *cur = cfg->cfgnode_list->next;
    assert(cur != cfg->cfgnode_list);
    CFGnode *cfgnode = (CFGnode *)cur->data;
    cfgnode->out_fact = set_create();
    cfgnode->in_fact = set_create();
    for (ListNode *cur_exp = exp_list_head->next; cur_exp != exp_list_head; cur_exp = cur_exp->next)
    {
        Exp *exp = (Exp *)cur_exp->data;
        cfgnode->out_fact = set_add(cfgnode->out_fact, exp);
    }
    Set *standard_outfact = cfgnode->out_fact;
    cur = cur->next;
    while (cur != cfg->cfgnode_list)
    {
        cfgnode = (CFGnode *)cur->data;
        cfgnode->in_fact = set_create();
        cfgnode->out_fact = set_copy(standard_outfact);
        cur = cur->next;
    }
    cfg->entry_node->out_fact = set_create();
    cfg->exit_node->in_fact = set_create();
    cfg->exit_node->out_fact = set_create();
}

static int transfer_node(CFGnode *cfgnode)
{
    if (cfgnode->type == ENTRY)
        return 1;
    if (cfgnode->type == EXIT)
        return 0;
    if (cfgnode->visited == 0)
        return 0;
    // 计算 infact
    ListNode *pre_node = cfgnode->predecessors->next;
    assert(pre_node != cfgnode->predecessors);
    CFGnode *pre = (CFGnode *)pre_node->data;
    Set *infact = set_copy(pre->out_fact);
    pre_node = pre_node->next;
    while (pre_node != cfgnode->predecessors)
    {
        pre = (CFGnode *)pre_node->data;
        Set *newfact = set_intersect(infact, pre->out_fact, exp_equal);
        infact = set_teardown(infact);
        infact = newfact;
        pre_node = pre_node->next;
    }

    IR *stmt = cfgnode->stmt;
    Set *outfact = set_copy(infact);
    Operand *left_var = NULL;
    if (stmt->type == ASSIGN_IR)
    {
        left_var = stmt->assign_ir.left;
    }
    else if (stmt->type == BINARY_IR)
    {
        left_var = stmt->binary_ir.left;
    }

    if (left_var != NULL)
        for (ListNode *exp_node = left_var->attached_exp->next; exp_node != left_var->attached_exp; exp_node = exp_node->next)
        {
            Exp *exp = (Exp *)exp_node->data;
            outfact = set_sub_data(outfact, exp, exp_equal);
        }

    // 处理带表达式的语句
    if (stmt->type == BINARY_IR)
    {
        Exp *exp = stmt->binary_ir.exp;
        if (set_contains(outfact, exp, exp_equal) != NULL)
        {
        }
        else
        {
            // 从 a := b op c 变成
            // exp1 := b op c
            // a := exp1
            outfact = set_add(outfact, exp);
            cfgnode->stmt->binary_ir.exp_var = exp->exp_var;
        }
    }

    int changed = set_equal(outfact, cfgnode->out_fact) == 1 ? 0 : 1;
    cfgnode->in_fact = set_teardown(cfgnode->in_fact);
    cfgnode->in_fact = infact;
    cfgnode->out_fact = set_teardown(cfgnode->out_fact);
    cfgnode->out_fact = outfact;
    print_fact(cfgnode);
    printf("exit with %d\n", changed);
    return changed;
}

static void solver(CFG *cfg)
{
    Queue *worklist = queue_create();

    // 将除exit以外的所有结点加入队列
    queue_push(worklist, cfg->entry_node);

    for (ListNode *cur = cfg->cfgnode_list->next; cur != cfg->cfgnode_list; cur = cur->next)
    {
        CFGnode *cfgnode = (CFGnode *)cur->data;
        queue_push(worklist, cfgnode);
    }

    while (!queue_empty(worklist))
    {
        CFGnode *cfgnode = (CFGnode *)queue_pop(worklist);

        if (transfer_node(cfgnode))
        {

            for (ListNode *succ = cfgnode->successors->next; succ != cfgnode->successors; succ = succ->next)
            {
                CFGnode *succ_cfgnode = (CFGnode *)succ->data;
                if (!queue_contains(worklist, succ_cfgnode))
                {
                    queue_push(worklist, succ_cfgnode);
                }
            }
        }
    }

    queue_teardown(worklist);
}

static void print_fact(CFGnode *cfgnode)
{

    Set *infact = cfgnode->in_fact;
    printf("%d in: ", cfgnode->stmt->index);
    for (ListNode *cur = infact->head->next; cur != infact->head; cur = cur->next)
    {
        Exp *exp = (Exp *)cur->data;
        printf("exp %s %d %s ", exp->left->name, exp->type, exp->right->name);
    }
    printf("\n%d out: ", cfgnode->stmt->index);
    Set *outfact = cfgnode->out_fact;
    for (ListNode *cur = outfact->head->next; cur != outfact->head; cur = cur->next)
    {
        Exp *exp = (Exp *)cur->data;
        printf("exp %s %d %s ", exp->left->name, exp->type, exp->right->name);
    }
    printf("\n");
}

static void print_cfg_fact(CFG *cfg)
{
    for (ListNode *cur = cfg->cfgnode_list->next; cur != cfg->cfgnode_list; cur = cur->next)
    {
        CFGnode *cfgnode = (CFGnode *)cur->data;
        print_fact(cfgnode);
    }
}

static ListNode *common_subexpression_elimination(CFG *cfg)
{
    ListNode *ir_list = list_create();
    for (ListNode *cur = cfg->cfgnode_list->next; cur != cfg->cfgnode_list; cur = cur->next)
    {
        CFGnode *cfgnode = (CFGnode *)cur->data;

        if (cfgnode->stmt->type == BINARY_IR)
        {

            Exp *search = NULL;
            search = set_contains(cfgnode->in_fact, cfgnode->stmt->binary_ir.exp, exp_equal);
            if (search != NULL)
            {
                // 若该语句的infact中含有其对应的表达式，说明该表达式为公共表达式，可以进行替换
                IR *new_stmt = ir_create(ASSIGN_IR);
                new_stmt->assign_ir.left = cfgnode->stmt->binary_ir.left;
                new_stmt->assign_ir.right = search->exp_var;
                new_stmt->cfg_node = cfgnode;
                free(cfgnode->stmt);
                cfgnode->stmt = new_stmt;
            }
            /*else
            {
                // 若不含有表达式，则
                // 从 a := b op c 变成
                // exp1 := b op c
                // a := exp1
                printf("fuck\n");
                assert(cfgnode->stmt->binary_ir.exp_var != NULL);
                IR *new_stmt = ir_create(ASSIGN_IR);
                new_stmt->assign_ir.left = cfgnode->stmt->binary_ir.left;
                new_stmt->assign_ir.right = cfgnode->stmt->binary_ir.exp_var;
                new_stmt->cfg_node = cfgnode;
                cfgnode->stmt->binary_ir.left = cfgnode->stmt->binary_ir.exp_var;

                ir_list = list_append_by_data(ir_list, cfgnode->stmt);
                ir_list = list_append_by_data(ir_list, new_stmt);
            }*/
        }

        ir_list = list_append_by_data(ir_list, cfgnode->stmt);
    }
    return ir_list;
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

void avilable_expressions_analysis()
{
    for (ListNode *cfg_node = cfgs_list_head->next; cfg_node != cfgs_list_head; cfg_node = cfg_node->next)
    {
        CFG *cfg = (CFG *)cfg_node->data;
        init(cfg);
        solver(cfg);
        // print_cfg_fact(cfg);
        common_subexpression_elimination(cfg);
        fact_teardown(cfg);
    }
}