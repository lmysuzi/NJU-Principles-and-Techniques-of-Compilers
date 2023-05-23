#include "constant_propagation.h"

static void print_fact(CFGnode *cfgnode)
{

    Set *infact = cfgnode->in_fact;
    printf("%d in: ", cfgnode->stmt->index);
    for (ListNode *cur = infact->head->next; cur != infact->head; cur = cur->next)
    {
        CPFact *fact = (CPFact *)cur->data;
        printf("fact %s %d %d ", fact->op->name, fact->val.type, fact->val.type == CONSTANT ? fact->val.val : 0);
    }
    printf("\n%d out: ", cfgnode->stmt->index);
    Set *outfact = cfgnode->out_fact;
    for (ListNode *cur = outfact->head->next; cur != outfact->head; cur = cur->next)
    {
        CPFact *fact = (CPFact *)cur->data;
        printf("fact %s %d %d ", fact->op->name, fact->val.type, fact->val.type == CONSTANT ? fact->val.val : 0);
    }
    printf("\n");
}

static int cpfact_equal(void *a, void *b)
{
    if (a == b)
        return 1;
    CPFact *ac = (CPFact *)a;
    CPFact *bc = (CPFact *)b;
    if (ac->op == bc->op)
        return 1;
    return 0;
}

static init(const CFG *cfg)
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

static Value meet_value(const Value a, const Value b)
{
    Value ans;
    if (a.type == NAC || b.type == NAC)
        ans.type = NAC;
    else if (a.type == CONSTANT && b.type == CONSTANT)
    {
        if (a.val == b.val)
        {
            ans.type = CONSTANT;
            ans.val = a.val;
        }
        else
            ans.type = NAC;
    }
    // 存在undef
    else if (a.type == CONSTANT)
    {
        ans.type = CONSTANT;
        ans.val = a.val;
    }
    else if (b.type == CONSTANT)
    {
        ans.type = CONSTANT;
        ans.val = b.val;
    }
    else
        ans.type = UNDEF;

    return ans;
}

static void meet_into(Set *fact, Set *target_fact)
{
    for (ListNode *cur = fact->head->next; cur != fact->head; cur = cur->next)
    {
        CPFact *cpfact = (CPFact *)cur->data;
        CPFact *target_cpfact = NULL;
        // 查找目标fact里面是否有该变量
        for (ListNode *target_cur = target_fact->head->next; target_cur != target_fact->head; target_cur = target_cur->next)
        {
            CPFact *search_cpfact = (CPFact *)target_cur->data;
            if (search_cpfact->op == cpfact->op)
            {
                target_cpfact = search_cpfact;
                break;
            }
        }
        // 目标target有该变量
        if (target_cpfact != NULL)
        {
            Value val = meet_value(cpfact->val, target_cpfact->val);
            assert(val.type == UNDEF);
            target_cpfact->val = val;
        }
        // 无
        else
        {
            target_fact = set_add(target_fact, cpfact);
        }
    }
}

static CPFact *cpfact_create(Operand *op, int type, int val)
{
    CPFact *cpfact = (CPFact *)malloc(sizeof(CPFact));
    Value value;
    value.type = type;
    value.val = val;
    cpfact->val = value;
    cpfact->op = op;
    return cpfact;
}

static Value evaluate_assign(IR *stmt, Set *infact)
{
    Value ans;
    if (stmt->assign_ir.right->type == IMMEDIATE_OP)
    {
        ans.type = CONSTANT;
        ans.val = stmt->assign_ir.right->val;
        return ans;
    }
    CPFact temp;
    temp.op = stmt->assign_ir.right;

    CPFact *search = (CPFact *)set_contains(infact, &temp, cpfact_equal);
    if (search == NULL)
        ans.type = UNDEF;
    else
    {
        ans = search->val;
    }
    return ans;
}

static int compute(int type, int a, int b)
{
    int ans;
    switch (type)
    {
    case ADD:
        ans = a + b;
        break;
    case SUB:
        ans = a - b;
        break;
    case MUL:
        ans = a * b;
        break;
    case DIV:
        ans = a / b;
        break;
    default:
        assert(0);
        break;
    }
    return ans;
}

static Value evaluate_binary(IR *stmt, Set *infact)
{
    Value ans;
    Value left, right;
    if (stmt->binary_ir.right1->type == IMMEDIATE_OP)
    {
        left.type = CONSTANT;
        left.val = stmt->binary_ir.right1->val;
    }
    else
    {
        CPFact temp;
        temp.op = stmt->binary_ir.right1;
        CPFact *search = (CPFact *)set_contains(infact, &temp, cpfact_equal);
        if (search == NULL)
            left.type = UNDEF;
        else
            left = search->val;
    }
    if (stmt->binary_ir.right2->type == IMMEDIATE_OP)
    {
        right.type = CONSTANT;
        right.val = stmt->binary_ir.right2->val;
    }
    else
    {
        CPFact temp;
        temp.op = stmt->binary_ir.right2;
        CPFact *search = (CPFact *)set_contains(infact, &temp, cpfact_equal);
        if (search == NULL)
            right.type = UNDEF;
        else
            right = search->val;
    }

        if (left.type == UNDEF || right.type == UNDEF)
        ans.type = UNDEF;
    else if (left.type == CONSTANT && right.type == CONSTANT)
    {
        if (right.val == 0 && stmt->binary_ir.exp->type == DIV)
            ans.type = UNDEF;
        else
        {
            ans.type = CONSTANT;
            ans.val = compute(stmt->binary_ir.exp->type, left.val, right.val);
        }
    }
    else if (left.type == NAC || right.type == NAC)
    {
        if (right.type == CONSTANT && right.val == 0 && stmt->binary_ir.exp->type == DIV)
            ans.type = UNDEF;
        else
            ans.type = NAC;
    }
    else
        assert(0);

    return ans;
}

static int transfer_node(CFGnode *cfgnode)
{
    if (cfgnode->type == EXIT)
        return 1;
    if (cfgnode->type == ENTRY)
        return 0;

    // 计算infact
    Set *infact = set_create();

    for (ListNode *pre_node = cfgnode->predecessors->next; pre_node != cfgnode->predecessors; pre_node = pre_node->next)
    {
        CFGnode *pre = (CFGnode *)pre_node->data;
        meet_into(pre->out_fact, infact);
    }

    // 复制outfact
    Set *outfact = set_copy(infact);

    IR *stmt = cfgnode->stmt;
    // 特殊处理param
    if (stmt->type == PARAM_IR)
    {
        CPFact *cpfact = cpfact_create(stmt->param_ir.param_var, NAC, 0);
        outfact = set_add(outfact, cpfact);
    }

    // 处理赋值语句
    Operand *def = NULL;
    Value value;

    if (stmt->type == ASSIGN_IR)
    {
        def = stmt->assign_ir.left;
        value = evaluate_assign(stmt, infact);
    }
    else if (stmt->type == BINARY_IR)
    {
        def = stmt->binary_ir.left;
        value = evaluate_binary(stmt, infact);
    }
    else if (stmt->type == READ_IR)
    {
        def = stmt->read_ir.read_var;
        value.type = NAC;
    }
    else if (stmt->type == CALL_IR)
    {
        def = stmt->call_ir.left;
        value.type = NAC;
    }

    if (def != NULL && def->type == VAR_OP)
    {
        CPFact *gen = cpfact_create(def, value.type, value.val);

        if (value.type == UNDEF)
        {
            outfact = set_sub_data(outfact, gen, cpfact_equal);
            free(gen);
        }
        else
        {
            CPFact *search = (CPFact *)set_contains(outfact, gen, cpfact_equal);
            // outfact没有对应变量的值
            if (search == NULL)
                outfact = set_add(outfact, gen);
            else
            {
                search->val = value;
                free(gen);
            }
        }
    }

    // 处理结果
    int changed = set_equal(infact, cfgnode->in_fact) == 1 ? 0 : 1;
    cfgnode->out_fact = set_teardown(cfgnode->out_fact);
    cfgnode->out_fact = outfact;
    cfgnode->in_fact = set_teardown(cfgnode->in_fact);
    cfgnode->in_fact = infact;

    print_fact(cfgnode);
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

void constant_propagation_analysis()
{
    for (ListNode *cfg_node = cfgs_list_head->next; cfg_node != cfgs_list_head; cfg_node = cfg_node->next)
    {
        CFG *cfg = (CFG *)cfg_node->data;
        init(cfg);
        solver(cfg);
        // print_cfg_fact(cfg);

        fact_teardown(cfg);
    }
}