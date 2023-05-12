#include "operand.h"

Operand *operand_create(char *text)
{
    Operand *op = (Operand *)malloc(sizeof(Operand));
    assert(op != NULL);

    if (text[0] == '#')
    {
        op->type = IMMEDIATE_OP;
        op->name = NULL;
    }
    else
    {
        op->type = VAR_OP;
        int index = 0;
        if (text[0] == '&' || text[0] == '*')
            index++;
        op->name = (char *)malloc(strlen(text) + 1);
        strcpy(op->name, text + index);
    }
    return op;
}