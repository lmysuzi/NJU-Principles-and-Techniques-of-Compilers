#ifndef __CONSTANT_PROPAGATION_H__
#define __CONSTANT_PROPAGATION_H__

#include "data_structure.h"
#include "operand.h"
#include "ir.h"
#include "cfg.h"

typedef struct Value
{
    int val;
    enum
    {
        NAC = 1,
        CONSTANT,
        UNDEF,
    } type;
} Value;

typedef struct CPFact
{
    Operand *op;
    Value val;
} CPFact;

void constant_propagation_analysis();

#endif