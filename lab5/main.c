#include <stdio.h>
#include "ir.h"
#include "bb.h"
#include "operand.h"
#include "cfg.h"
#include "avilable_expressions_analysis.h"
#include "live_variable_analysis.h"
#include "constant_propagation.h"

FILE *input_file = NULL;
FILE *output_file = NULL;

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
        if (!(output_file = fopen(argv[2], "w")))
        {
            perror(argv[2]);
            return 1;
        }
        init();
        ir_extract(input_file);
        cfgs_build();
        // avilable_expressions_analysis();
        // live_variable_analysis();
        constant_propagation_analysis();
        cfgs_output(output_file);
        fclose(output_file);
        fclose(input_file);
        // build_BBs(ir_list);
    }
}