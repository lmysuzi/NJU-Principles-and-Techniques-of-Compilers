#ifndef __AVILABLE_EXPRESSIONS_ANALYSIS_H__
#define __AVILABLE_EXPRESSIONS_ANALYSIS_H__

#include "cfg.h"
#include "ir.h"

typedef struct ExpFact
{
    // 表达式
    Exp *exp;
    // 该表达式的值所对应的变量
    // 当op为空时，表示该fact是初始化的fact，无实际作用
    Operand *op;
} ExpFact;

void avilable_expressions_analysis();

#endif