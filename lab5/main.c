#include <stdio.h>
#include "ir.h"
#include "bb.h"
#include "operand.h"

FILE *input_file = NULL;

void initial()
{
    operand_initial();
    BB_initial();
}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        if (!(input_file = fopen(argv[1], "r")))
        {
            perror(argv[1]);
            return 1;
        }
        initial();
        ListNode *ir_list = list_create();
        ir_list = ir_extract(input_file, ir_list);
        build_BBs(ir_list);
    }
}