#include "cfg.h"

ListNode *cfgs_list_head;

void cfg_init()
{
    cfgs_list_head = list_create();
}

static CFGnode *cfg_node_create(int type, IR *ir)
{
    CFGnode *cfgnode = (CFGnode *)malloc(sizeof(CFGnode));
    cfgnode->type = type;
    cfgnode->in_fact = NULL;
    cfgnode->out_fact = NULL;
    cfgnode->predecessors = list_create();
    cfgnode->successors = list_create();
    cfgnode->stmt = ir;
    cfgnode->dead = 0;
    cfgnode->visited = 0;
    return cfgnode;
}

static CFG *cfg_create()
{
    CFG *cfg = (CFG *)malloc(sizeof(CFG));
    cfg->entry_node = cfg_node_create(ENTRY, NULL);
    cfg->exit_node = cfg_node_create(EXIT, NULL);
    cfg->cfgnode_list = list_create();
    return cfg;
}

static void cfg_add_edge(CFGnode *root, CFGnode *target)
{
    root->successors = list_append_by_data(root->successors, target);
    target->predecessors = list_append_by_data(target->predecessors, root);
}

static CFGnode *search_label(char *name)
{
    for (ListNode *cur = label_list_head->next; cur != label_list_head; cur = cur->next)
    {
        IR *ir = (IR *)cur->data;
        assert(ir != NULL && ir->type == LABEL_IR);
        if (strcmp(name, ir->label_ir.label_name) == 0)
        {
            return ir->cfg_node;
        }
    }
    assert(0);
    return NULL;
}

// 建立cfg图
static CFG *cfg_build(ListNode *ir_list)
{
    CFG *cfg = cfg_create();
    CFGnode *last_cfg_node = cfg->entry_node;

    // 不处理跳转
    for (ListNode *cur = ir_list->next; cur != ir_list; cur = cur->next)
    {
        IR *ir = (IR *)cur->data;

        CFGnode *cur_cfg_node = cfg_node_create(NORMAL, ir);
        cfg->cfgnode_list = list_append_by_data(cfg->cfgnode_list, cur_cfg_node);
        ir->cfg_node = cur_cfg_node;

        if (last_cfg_node != NULL)
            cfg_add_edge(last_cfg_node, cur_cfg_node);

        switch (ir->type)
        {
        case GOTO_IR:
            last_cfg_node = NULL;
            break;
        case RETURN_IR:
            cfg_add_edge(cur_cfg_node, cfg->exit_node);
            last_cfg_node = NULL;
            break;
        default:
            last_cfg_node = cur_cfg_node;
            break;
        }
    }

    for (ListNode *cur = ir_list->next; cur != ir_list; cur = cur->next)
    {
        IR *ir = (IR *)cur->data;
        if (ir->type == GOTO_IR || ir->type == CONDITIONAL_GOTO_IR)
        {
            char *name = ir->type == GOTO_IR ? ir->goto_ir.label_name : ir->conditional_goto_ir.label_name;
            CFGnode *cfgnode = search_label(name);
            assert(cfgnode != NULL);
            cfg_add_edge(ir->cfg_node, cfgnode);
        }
    }

    return cfg;
}

void cfgs_build()
{
    for (ListNode *cur = func_list_head->next; cur != func_list_head; cur = cur->next)
    {
        ListNode *single_func = (ListNode *)cur->data;
        CFG *cfg = cfg_build(single_func);
        cfgs_list_head = list_append_by_data(cfgs_list_head, cfg);
    }
}

void cfgs_output(FILE *file)
{
    for (ListNode *cur_cfg = cfgs_list_head->next; cur_cfg != cfgs_list_head; cur_cfg = cur_cfg->next)
    {
        CFG *cfg = (CFG *)cur_cfg->data;
        for (ListNode *cur = cfg->cfgnode_list->next; cur != cfg->cfgnode_list; cur = cur->next)
        {
            CFGnode *cfgnode = (CFGnode *)cur->data;
            if (cfgnode->dead == 0 && cfgnode->visited == 1)
                fprintf_ir(file, cfgnode->stmt);
        }
    }
}