#include <stdio.h>
#include "ir.h"
#include "bb.h"
#include "operand.h"
#include "cfg.h"

FILE *input_file = NULL;

void init()
{
    operand_initial();
    BB_initial();
    ir_init();
    cfg_init();
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
        init();
        ir_extract(input_file);
        cfgs_build();

        // build_BBs(ir_list);
    }
}