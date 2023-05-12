#include <stdio.h>
#include "ir.h"

FILE *input_file = NULL;

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        if (!(input_file = fopen(argv[1], "r")))
        {
            perror(argv[1]);
            return 1;
        }
        ListNode *ir_list = list_create();
        ir_list = ir_extract(input_file, ir_list);
    }
}